/*
 *  Ship velocity limits, vector arithmetic and orientation geometry.
 *
 *  Retail object 0x40A3A0-0x40D6AF (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x40A3A0 */
short get_ship_max_velocity(short obj)
{
    short velocity = g_aObjectTypeData[
        g_acObjectType[obj]].maximumVelocity;

    if (g_asShipSide[obj] != g_asShipSide[0] &&
        g_aeObjectClass[obj] == OBJECT_CLASS_SHIP) {
        velocity = (short)(
            (int)MinShort(150, MaxShort(80, GetAdaptiveTurnRate())) *
            velocity / 100);
    }
    if (g_asPilotLevel[obj] == RATING_ACE_SPIRIT &&
        g_asShipSide[obj] == SIDE_KILRATHI)
        return velocity + velocity / 3;
    else
        return velocity;
}

/* Function start: 0x40A481 */
void recalc_max_velocity(short ship)
{
    short oldVelocity;

    oldVelocity = g_asShipMaximumVelocity[ship];
    if (g_anShipFuel[ship] <= 0) {
        g_asShipMaximumVelocity[ship] = 5;
    } else {
        g_asShipMaximumVelocity[ship] =
            (short)(((4 - (int)g_acShipIonDriveDamage[ship]) *
                     (int)get_ship_max_velocity(ship)) >> 2);
    }
    if (g_asShipMaximumVelocity[ship] != oldVelocity)
        celerate(ship, 0);
}

/* Function start: 0x40A51D */
void drain_fuel(short ship, short amount)
{
    g_anShipFuel[ship] -= (int)amount;
    if (g_anShipFuel == 0)
        recalc_max_velocity(ship);
}

/* Function start: 0x40A556 */
void damage_ion_drive(short ship, short amount, short maximum)
{
    g_acShipIonDriveDamage[ship] = (signed char)MaxShort(
        0, MinShort((short)(g_acShipIonDriveDamage[ship] + amount),
                    maximum));
    recalc_max_velocity(ship);
}

/* Function start: 0x40A5A0 */
int GetShipAccelerationRate(short ship)
{
    int adaptiveScale;
    int acceleration;

    acceleration = g_aObjectTypeData[
        g_acObjectType[ship]].acceleration;
    if (g_asShipSide[ship] != g_asShipSide[0] &&
        g_aeObjectClass[ship] == OBJECT_CLASS_SHIP) {
        adaptiveScale =
            (int)MinShort(150, MaxShort(80, GetAdaptiveTurnRate())) << 8;
        acceleration = MultiplyFixed(acceleration, adaptiveScale);
        acceleration = DivideFixed(acceleration, 100 << 8);
    }
    if (g_asPilotLevel[ship] == RATING_ACE_SPIRIT &&
        g_asShipSide[ship] == SIDE_KILRATHI)
        return acceleration + acceleration / 3;
    else
        return acceleration;
}

/* Function start: 0x40A691 */
void point_at(short obj, FixedVector point)
{
    FixedVector direction;

    ComputeVectorDelta(&g_aShipPosition[obj], &point, &direction);
    shrink_vector(&direction);
    g_aShipForwardVector[obj] = direction;
    fix_objects_ijk(obj);
}

/* Function start: 0x40A6F7 */
void look_at(short obj)
{
    point_at(EYE_OBJECT, g_aShipPosition[obj]);
}

/* Function start: 0x40A725 */
void position_relative(FixedVector *position, FixedVector direction,
                       short distance)
{
    if (distance != 0) {
        NormalizeFixedVector(&direction);
#ifdef SDL_PORT
        ScaleFixedVector(&direction, (int)distance * 0x100, &direction);
#else
        ScaleFixedVector(&direction, (int)distance << 8, &direction);
#endif
        AddFixedVectors(position, &direction, position);
    }
}

/* Function start: 0x40A774 */
void position_relative_ijk(FixedVector *position, short obj,
                           short right, short up, short forward)
{
    *position = g_aShipPosition[obj];
    position_relative(position, g_aShipRightVector[obj], right);
    position_relative(position, g_aShipUpVector[obj], up);
    position_relative(position, g_aShipForwardVector[obj], forward);
}

/* Function start: 0x40A81B */
short FixedToShortSaturating(int value)
{
    if (value < -0x7fff00)
        return -0x7fff;
    else if (value > 0x7fff00)
        return 0x7fff;
    else
        return (short)(value >> 8);
}

/* Function start: 0x40A867 */
int MinInt(int a, int b)
{
    if (b < a)
        return b;
    else
        return a;
}

/* Function start: 0x40A893 */
int MaxInt(int a, int b)
{
    if (b > a)
        return b;
    else
        return a;
}

/* Function start: 0x40A8BF */
int AbsInt(int v)
{
    if (v < 0)
        return -v;
    else
        return v;
}

/* Function start: 0x40A8EB */
unsigned int SignFixed(int v)
{
    if (v < 0)
        return 0xffffff00;
    if (v > 0)
        return 0x100;
    return 0;
}

/* Function start: 0x40A925 */
/* Normalise a degree value into (-180, 180]. 0x168 == 360, 0xB4 == 180. */
short WrapDegrees(short degrees)
{
    degrees %= 0x168;
    if (degrees < -0xb4)
        degrees += 0x168;
    if (0xb4 < degrees)
        degrees -= 0x168;
    return degrees;
}

/* Function start: 0x40A981 */
short equ_vector(const FixedVector *left, const FixedVector *right)
{
    return left->x == right->x && left->y == right->y &&
           left->z == right->z;
}

void negate_vector(FixedVector *vector);

/* Function start: 0x40A9D1 */
short IsPairEqualityDifferentFromFlag(const unsigned int *values)
{
    return ((values[1] == values[0]) == values[2]) == 0;
}

/* Function start: 0x40AA0B */
void zero_vector(FixedVector *vector)
{
    vector->x = vector->y = vector->z = 0;
}

/* Function start: 0x40AA37 */
void negate_vector(FixedVector *vector)
{
    vector->x = -vector->x;
    vector->y = -vector->y;
    vector->z = -vector->z;
}

/* Function start: 0x40AA6A */
void AddFixedVectors(const FixedVector *left, const FixedVector *right,
                     FixedVector *sum)
{
    sum->x = right->x + left->x;
    sum->y = right->y + left->y;
    sum->z = right->z + left->z;
}

/* Function start: 0x40AAA8 */
void SubtractFixedVectors(FixedVector *left, FixedVector *right,
                          FixedVector *difference)
{
    difference->x = left->x - right->x;
    difference->y = left->y - right->y;
    difference->z = left->z - right->z;
}

/* Function start: 0x40AAE6 */
void ComputeVectorDelta(FixedVector *from, FixedVector *to, FixedVector *delta)
{
    delta->x = to->x - from->x;
    delta->y = to->y - from->y;
    delta->z = to->z - from->z;
}

/* Function start: 0x40AB24 */
void ScaleFixedVector(FixedVector *vector, int scale, FixedVector *result)
{
    result->x = MultiplyFixed(vector->x, scale);
    result->y = MultiplyFixed(vector->y, scale);
    result->z = MultiplyFixed(vector->z, scale);
}

/* Function start: 0x40AB78 */
void divide_vector(FixedVector *vector, int divisor, FixedVector *result)
{
    result->x = DivideFixed(vector->x, divisor);
    result->y = DivideFixed(vector->y, divisor);
    result->z = DivideFixed(vector->z, divisor);
}

/* Function start: 0x40ABCC */
short ChooseRandomSignedMagnitude(short minimum, short maximum,
                                  short allowNegative)
{
    short value = RandomInRange(minimum, maximum);

    if (allowNegative != 0 && RandomInRange(0, 1) != 0)
        value = -value;
    return value;
}

/* Function start: 0x40AC24 */
void MakeRandomVectorFixed(short minimum, short maximum, FixedVector *vector)
{
#ifdef SDL_PORT
    vector->x = ChooseRandomSignedMagnitude(minimum, maximum, 1) * 0x100;
    vector->y = ChooseRandomSignedMagnitude(minimum, maximum, 1) * 0x100;
    vector->z = ChooseRandomSignedMagnitude(minimum, maximum, 1) * 0x100;
#else
    vector->x = ChooseRandomSignedMagnitude(minimum, maximum, 1) << 8;
    vector->y = ChooseRandomSignedMagnitude(minimum, maximum, 1) << 8;
    vector->z = ChooseRandomSignedMagnitude(minimum, maximum, 1) << 8;
#endif
}

/* Function start: 0x40AC88 */
void FillFixedVectorWithRandomComponents(short limit, FixedVector *vector)
{
    MakeRandomVectorFixed(0, limit, vector);
}

/* Function start: 0x40ACA5 */
void random_radial(const FixedVector *center, short radius,
                   FixedVector *position)
{
    FixedVector offset;

    FillFixedVectorWithRandomComponents(
        RandomBelowOrEqual(radius), &offset);
    AddFixedVectors(center, &offset, position);
}

/* Function start: 0x40ACE0 */
void rectangular_to_spherical(const FixedVector *rectangular,
                              SphericalVector *spherical)
{
    spherical->radius = Vector_magnitude(rectangular);
    if (spherical->radius == 0)
        return;
    spherical->yaw = (short)ArcCos(
        DivideFixed(rectangular->z,
                    PlanarMagnitude(rectangular->x, rectangular->z)));
    if (rectangular->x < 0)
        spherical->yaw = -spherical->yaw;
    spherical->pitch = (short)(ArcCos(
        DivideFixed(rectangular->y, spherical->radius)) - 90);
}

/* Function start: 0x40AD89 */
/* No inbound reference is known in the shipped executable; this routine is
 * believed unreachable. */
void ConvertFixedVectorToShortVector(const FixedVector *source,
                                     ShortVector *destination)
{
    destination->x = (short)(source->x >> 8);
    destination->y = (short)(source->y >> 8);
    destination->z = (short)(source->z >> 8);
}

/* Function start: 0x40ADC2 */
int dot_product(const FixedVector *left, const FixedVector *right)
{
    return MultiplyFixed(left->x, right->x) +
           MultiplyFixed(left->y, right->y) +
           MultiplyFixed(left->z, right->z);
}

/* Function start: 0x40AE18 */
short vector_angle(FixedVector left, FixedVector right)
{
    short cosine;

    NormalizeFixedVector(&left);
    NormalizeFixedVector(&right);
    cosine = (short)dot_product(&left, &right);
    return (short)((cosine * 100) / 0x100);
}

/* Function start: 0x40AE70 */
void vector_cross_product(const FixedVector *left, const FixedVector *right,
                          FixedVector *product)
{
    product->x = MultiplyFixed(left->y, right->z) -
                 MultiplyFixed(left->z, right->y);
    product->y = MultiplyFixed(left->z, right->x) -
                 MultiplyFixed(left->x, right->z);
    product->z = MultiplyFixed(left->x, right->y) -
                 MultiplyFixed(left->y, right->x);
}

/* Function start: 0x40AF18 */
short NormalizeFixedVector(FixedVector *vector)
{
    int magnitude = Vector_magnitude(vector);

    if (magnitude != 0) {
        vector->x = DivideFixed(vector->x, magnitude);
        vector->y = DivideFixed(vector->y, magnitude);
        vector->z = DivideFixed(vector->z, magnitude);
        return 1;
    }
    return 0;
}

/* Function start: 0x40AF99 */
int vector_length_in_dir(const FixedVector *vector,
                         const FixedVector *direction)
{
    FixedVector normalized;
    int projection;

    normalized = *vector;
    NormalizeFixedVector(&normalized);
    projection = MultiplyFixed(Vector_magnitude(vector),
                           dot_product(direction, &normalized));
    return projection;
}

/* Function start: 0x40AFFA */
void vector_component_in_dir(const FixedVector *vector,
                             const FixedVector *direction,
                             FixedVector *component)
{
    int length;

    length = vector_length_in_dir(vector, direction);
    ScaleFixedVector((FixedVector *)direction, length, component);
}

void vector_cross_product(const FixedVector *left, const FixedVector *right, FixedVector *product);
void vector_component_in_dir(const FixedVector *vector, const FixedVector *direction, FixedVector *component);
void transform_to_objects_frame(const FixedVector *source, FixedVector *destination, short obj);
void ScaleFixedVector(FixedVector *vector, int scale, FixedVector *result);
int vector_length_in_dir(const FixedVector *vector, const FixedVector *direction);
void copy_frame(short source, short destination);

/* Function start: 0x40B02F */
void rotate_about_i(short angle, FixedVector *j, FixedVector *k)
{
    int original;
    int sine;
    int cosine;

    cosine = CosFixed(angle);
    sine = SinFixed(angle);
    original = j->x;
    j->x = MultiplyFixed(original, cosine) - MultiplyFixed(k->x, sine);
    k->x = MultiplyFixed(original, sine) + MultiplyFixed(k->x, cosine);
    original = j->y;
    j->y = MultiplyFixed(original, cosine) - MultiplyFixed(k->y, sine);
    k->y = MultiplyFixed(original, sine) + MultiplyFixed(k->y, cosine);
    original = j->z;
    j->z = MultiplyFixed(original, cosine) - MultiplyFixed(k->z, sine);
    k->z = MultiplyFixed(original, sine) + MultiplyFixed(k->z, cosine);
}

/* Function start: 0x40B17F */
void rotate_about_j(short angle, FixedVector *i, FixedVector *k)
{
    int cosine;
    int sine;
    int original;

    cosine = CosFixed(angle);
    sine = SinFixed(angle);
    original = i->x;
    i->x = MultiplyFixed(k->x, sine) + MultiplyFixed(original, cosine);
    k->x = MultiplyFixed(k->x, cosine) - MultiplyFixed(original, sine);
    original = i->y;
    i->y = MultiplyFixed(k->y, sine) + MultiplyFixed(original, cosine);
    k->y = MultiplyFixed(k->y, cosine) - MultiplyFixed(original, sine);
    original = i->z;
    i->z = MultiplyFixed(k->z, sine) + MultiplyFixed(original, cosine);
    k->z = MultiplyFixed(k->z, cosine) - MultiplyFixed(original, sine);
}

/* Function start: 0x40B2CF */
void rotate_about_k(short angle, FixedVector *i, FixedVector *j)
{
    int original;
    int sine;
    int cosine;

    cosine = CosFixed(angle);
    sine = SinFixed(angle);
    original = i->x;
    i->x = MultiplyFixed(original, cosine) - MultiplyFixed(j->x, sine);
    j->x = MultiplyFixed(original, sine) + MultiplyFixed(j->x, cosine);
    original = i->y;
    i->y = MultiplyFixed(original, cosine) - MultiplyFixed(j->y, sine);
    j->y = MultiplyFixed(original, sine) + MultiplyFixed(j->y, cosine);
    original = i->z;
    i->z = MultiplyFixed(original, cosine) - MultiplyFixed(j->z, sine);
    j->z = MultiplyFixed(original, sine) + MultiplyFixed(j->z, cosine);
}

/* Function start: 0x40B41F */
void init_ijk(short obj)
{
    g_aShipRightVector[obj].x =
        g_aShipUpVector[obj].y =
            g_aShipForwardVector[obj].z = 0x100;
    g_aShipRightVector[obj].y =
        g_aShipRightVector[obj].z =
            g_aShipUpVector[obj].x =
                g_aShipUpVector[obj].z =
                    g_aShipForwardVector[obj].x =
                        g_aShipForwardVector[obj].y = 0;
    g_anObjectPitchRotation[obj] =
        g_anObjectYawRotation[obj] =
            g_anObjectRollRotation[obj] = 0;
}

/* Function start: 0x40B550 */
void copy_frame(short source, short destination)
{
    g_aShipRightVector[destination] =
        g_aShipRightVector[source];
    g_aShipUpVector[destination] =
        g_aShipUpVector[source];
    g_aShipForwardVector[destination] =
        g_aShipForwardVector[source];
}

/* Function start: 0x40B5DF */
void fix_objects_ijk(short obj)
{
    vector_cross_product(&g_aShipUpVector[obj],
                         &g_aShipForwardVector[obj],
                         &g_aShipRightVector[obj]);
    vector_cross_product(&g_aShipForwardVector[obj],
                         &g_aShipRightVector[obj],
                         &g_aShipUpVector[obj]);
    NormalizeFixedVector(&g_aShipRightVector[obj]);
    NormalizeFixedVector(&g_aShipUpVector[obj]);
    NormalizeFixedVector(&g_aShipForwardVector[obj]);
}

/* Function start: 0x40B699 */
void transform_to_objects_frame(const FixedVector *source,
                                FixedVector *destination, short obj)
{
    destination->x = dot_product(source, &g_aShipRightVector[obj]);
    destination->y = dot_product(source, &g_aShipUpVector[obj]);
    destination->z = dot_product(source, &g_aShipForwardVector[obj]);
}

/* Function start: 0x40B706 */
void alter_pitch(short angle, short obj)
{
    rotate_about_i(angle, &g_aShipUpVector[obj],
                   &g_aShipForwardVector[obj]);
    fix_objects_ijk(obj);
}

/* Function start: 0x40B747 */
void alter_yaw(short angle, short obj)
{
    rotate_about_j(angle, &g_aShipRightVector[obj],
                   &g_aShipForwardVector[obj]);
    fix_objects_ijk(obj);
}

/* Function start: 0x40B788 */
void alter_roll(short angle, short obj)
{
    rotate_about_k(angle, &g_aShipRightVector[obj],
                   &g_aShipUpVector[obj]);
    fix_objects_ijk(obj);
}

/* Function start: 0x40B7C9 */
short distance_between_points(const FixedVector *from,
                              const FixedVector *to)
{
    FixedVector delta;

    ComputeVectorDelta((FixedVector *)from, (FixedVector *)to, &delta);
    return FixedToShortSaturating((int)Vector_magnitude(&delta));
}

/* Function start: 0x40B805 */
short distance_from_point(short obj, const FixedVector *point)
{
    ComputeVectorDelta(&g_aShipPosition[obj],
                       (FixedVector *)point, &g_vToTarget);
    return FixedToShortSaturating((int)Vector_magnitude(&g_vToTarget)) -
           g_asObjectCollisionRadius[obj];
}

/* Function start: 0x40B85C */
short distance_from_object(short obj, short other)
{
    return distance_from_point(obj, &g_aShipPosition[other]) -
           g_asObjectCollisionRadius[other];
}

/* Counts facing and range queries; nothing reads it. */
int g_nFacingRangeQueryCount = 0; /* 0x00492708 */
int g_anCloakVisibilityPattern[20] = {
    1, 0, 0, 0, 1, 0, 0, 1, 1, 0,
    0, 0, 1, 1, 1, 0, 0, 1, 1, 0
}; /* 0x00492710 */
int g_abObjectShapeOwned[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00492760 */

/* Function start: 0x40B898 */
void get_facing_range_from_point(short obj, const FixedVector *point)
{
    int cosine;

    g_nTargetRange =
        distance_from_point(obj, point) -
        g_asObjectCollisionRadius[obj];
    g_2_vNormalizedToTgt = g_vToTarget;
    NormalizeFixedVector(&g_2_vNormalizedToTgt);
    cosine = dot_product(&g_2_vNormalizedToTgt,
                         &g_aShipForwardVector[obj]);
    g_nFacingToTarget = (short)(((short)cosine * 100) >> 8);
    g_nFacingRangeQueryCount++;
}

/* Function start: 0x40B92F */
void get_facing_range_from_object(short obj, short other)
{
    int cosine;

    get_facing_range_from_point(obj, &g_aShipPosition[other]);
    g_nTargetRange -= g_asObjectCollisionRadius[other];
    negate_vector(&g_2_vNormalizedToTgt);
    cosine = dot_product(&g_2_vNormalizedToTgt,
                         &g_aShipForwardVector[other]);
    g_nTargetFacing = (short)(((short)cosine * 100) >> 8);
}

/* Function start: 0x40B9B5 */
short facing_to_object(short obj, FixedVector *point)
{
    FixedVector direction;
    int cosine;

    ComputeVectorDelta(&g_aShipPosition[obj], point, &direction);
    NormalizeFixedVector(&direction);
    cosine = dot_product(&direction, &g_aShipForwardVector[obj]);
    g_nFacingToTarget = (short)(((short)cosine * 100) >> 8);
    return g_nFacingToTarget;
}

/* Function start: 0x40BA2E */
short match_roll_orientation(short obj, short reference)
{
    FixedVector roll;
    short angle;

    roll.x = dot_product(&g_aShipUpVector[obj],
                         &g_aShipRightVector[reference]);
    roll.y = dot_product(&g_aShipUpVector[obj],
                         &g_aShipUpVector[reference]);
    roll.z = 0;
    NormalizeFixedVector(&roll);
    angle = (short)ArcCos(roll.y);
    if (roll.x >= 0)
        angle = 360 - angle;
    return WrapDegrees(angle);
}

/* Function start: 0x40BADB */
short set_ship_rotation_goals(short obj, short turnRate,
                            const FixedVector *direction,
                            short pointingMode,
                            short *yawGoal, short *pitchGoal)
{
    SphericalVector spherical;
    FixedVector localDirection;
    int magnitude;

    transform_to_objects_frame(direction, &localDirection, obj);
    if (pointingMode == 1) {
        rectangular_to_spherical(&localDirection, &spherical);
        if (spherical.radius == 0)
            return 1;
        if (spherical.yaw > 0)
            spherical.yaw = (short)(spherical.yaw - turnRate);
        else
            spherical.yaw = (short)(spherical.yaw + turnRate);
    } else {
        magnitude = Vector_magnitude(&localDirection);
        if (magnitude == 0)
            return 1;
        spherical.yaw = (short)ArcSin(DivideFixed(
            localDirection.x,
            PlanarMagnitude(localDirection.x, localDirection.z)));
        spherical.pitch = (short)-ArcSin(
            DivideFixed(localDirection.y, magnitude));
        if (localDirection.z < 0) {
            spherical.yaw = (short)-spherical.yaw;
            if (spherical.pitch > 0)
                spherical.pitch = (short)(spherical.pitch + 180);
            else
                spherical.pitch = (short)(spherical.pitch - 180);
        }
        if (spherical.pitch > 0)
            spherical.pitch = (short)(spherical.pitch - turnRate);
        else
            spherical.pitch = (short)(spherical.pitch + turnRate);
    }
    *yawGoal = WrapDegrees((short)-spherical.yaw);
    *pitchGoal = WrapDegrees((short)-spherical.pitch);
    return 0;
}

/* Function start: 0x40BC6B */
void point_ship(short obj, short turnRate, const FixedVector *direction)
{
    set_ship_rotation_goals(obj, turnRate, direction, 1,
                            &g_anYawGoal[obj],
                            &g_anPitchGoal[obj]);
    return;
}

/* Function start: 0x40BCA9 */
void point_ship_at_point(short obj, const FixedVector *point)
{
    FixedVector direction;

    ComputeVectorDelta(&g_aShipPosition[obj],
                       (FixedVector *)point, &direction);
    point_ship(obj, 0, &direction);
}

/* Function start: 0x40BCE8 */
void point_ship_at_object(short obj, short other)
{
    point_ship_at_point(obj, &g_aShipPosition[other]);
}

/* Function start: 0x40BD0E */
void point_capital_ship_at_object(short obj, short other)
{
    FixedVector direction;

    ComputeVectorDelta(&g_aShipPosition[other],
                       &g_aShipPosition[obj], &direction);
    point_ship(obj, 0, &direction);
}

/* Function start: 0x40BD58 */
void point_perpendicular_to_point(short obj, const FixedVector *point)
{
    point_ship_at_point(obj, point);
    if (g_anYawGoal[obj] < 0)
        g_anYawGoal[obj] += 90;
    else
        g_anYawGoal[obj] -= 90;
}

/* Function start: 0x40BDC2 */
void point_ship_behind_object(short obj, short other)
{
    FixedVector point = g_aShipPosition[other];

    position_relative(&point, g_aShipForwardVector[other],
                      (short)-(g_asObjectCollisionRadius[other] +
                               500));
    point_ship_at_point(obj, &point);
}

/* Function start: 0x40BE3A */
void point_ship_below_object(short obj, short other)
{
    FixedVector point = g_aShipPosition[other];

    position_relative(&point, g_aShipUpVector[other],
                      (short)(g_asObjectCollisionRadius[other] +
                              500));
    point_ship_at_point(obj, &point);
}

/* Function start: 0x40BEB0 */
void point_perpendicular(short obj, short other)
{
    point_perpendicular_to_point(obj, &g_aShipPosition[other]);
}

/* Function start: 0x40BED6 */
void point_parallel(short obj, short other)
{
    if (other != -1)
        point_ship(obj, 0, &g_aShipForwardVector[other]);
}

/* Function start: 0x40BF0B */
void NormalizeAndScaleVector(FixedVector *vector, int scale)
{
    NormalizeFixedVector(vector);
    ScaleFixedVector(vector, scale, vector);
}

/* Function start: 0x40BF36 */
void SetVectorFixedPoint(unsigned int *p, short v)
{
#ifdef SDL_PORT
    NormalizeAndScaleVector((FixedVector *)p, (int)v * 0x100);
#else
    NormalizeAndScaleVector((FixedVector *)p, (int)v << 8);
#endif
}

/* Function start: 0x40BF55 */
short IsPointWithinRange(FixedVector *from, FixedVector *to, short range)
{
    FixedVector delta;

    ComputeVectorDelta(from, to, &delta);
    return IsVectorWithinRange(&delta, range);
}

/* Function start: 0x40BF8C */
short check_for_collision(short obj)
{
    FixedVector *objectPosition;
    FixedVector *position;
    short candidate;
    short range;

    objectPosition = &g_aShipPosition[obj];
    position = g_aShipPosition;
    for (candidate = 0; candidate <= SPACE_LAST_MOVING_OBJECT;
         candidate++, position++) {
        if (candidate != obj &&
            g_aeObjectClass[candidate] >= OBJECT_CLASS_PROJECTILE) {
            ComputeVectorDelta(objectPosition, position,
                               &g_vCollisionDelta);
            range = (short)(
#ifdef SDL_PORT
                SdlGetBalancedCollisionRadius(candidate) +
                SdlGetBalancedCollisionRadius(obj));
#else
                g_asObjectCollisionRadius[candidate] +
                g_asObjectCollisionRadius[obj]);
#endif
            if (g_aeObjectClass[candidate] == OBJECT_CLASS_SHIP &&
                g_aeObjectClass[obj] == OBJECT_CLASS_SHIP)
                range >>= 1;
            if (IsVectorWithinRange(&g_vCollisionDelta,
                                    range) != 0)
                return candidate;
        }
    }
    return -1;
}

void remove_object(short obj);

/* Function start: 0x40C08C */
void position_child(short parent, short weapon, FixedVector *position)
{
    ShortVector *mount;

    mount = &((struct ShipLoadout *)g_aShipWeapons)[parent].slots[
        weapon].mount;
    position->x = g_aShipUpVector[parent].x * mount->y +
                  g_aShipForwardVector[parent].x * mount->z +
                  g_aShipRightVector[parent].x * mount->x +
                  g_aShipPosition[parent].x;
    position->y = g_aShipUpVector[parent].y * mount->y +
                  g_aShipRightVector[parent].y * mount->x +
                  g_aShipForwardVector[parent].y * mount->z +
                  g_aShipPosition[parent].y;
    position->z = g_aShipUpVector[parent].z * mount->y +
                  g_aShipForwardVector[parent].z * mount->z +
                  g_aShipRightVector[parent].z * mount->x +
                  g_aShipPosition[parent].z;
}

/* Function start: 0x40C1DA */
void child_object(short hardpoint, short child, short parent)
{
    position_child(parent, hardpoint, &g_aShipPosition[child]);
    g_acObjectOwner[child] = (signed char)parent;
}

/* Function start: 0x40C211 */
short get_ship_slot(void)
{
    short slot;

    for (slot = 1; slot <= 9; slot++) {
        if (g_aeObjectClass[slot] == OBJECT_CLASS_NULL)
            return slot;
    }
    return -1;
}

/* Function start: 0x40C266 */
short find_vacant_3d_object(void)
{
    short object;

    for (object = 10; object <= SPACE_LAST_MOVING_OBJECT; object++) {
        if (g_aeObjectClass[object] == OBJECT_CLASS_NULL) {
            g_asObjectScreenX[object] = (short)0x8001;
            return object;
        }
    }
    return -1;
}

/* Packet-release callee from the resource loader. */
void FreePacketAndClear(void *slot, unsigned short releaseFlags);

/* Function start: 0x40C2C9 */
void remove_object(short obj)
{
    short slot;

    if (obj == -1)
        return;
    g_asObjectScreenX[obj] = (short)0x8001;
    g_asObjectDistance[obj] = 0;
    if (obj == g_nNavPointerObject)
        g_nNavPointerObject = -1;
    if (obj == g_nYourWingman)
        g_nYourWingman = -1;
    for (slot = 0; slot < 20; slot++) {
        if (g_abHazardObjects[slot] == obj) {
            g_abHazardObjects[slot] = -1;
            break;
        }
    }
    if (obj < 10) {
        if (g_aeObjectClass[obj] >= OBJECT_CLASS_CAPITAL_SHIP ||
            (g_aeObjectClass[obj] == OBJECT_CLASS_SHIP &&
             g_asObjectType[obj] == 0x33))
#ifdef SDL_PORT
        {
            /* get_right_shape lends these ships a view packet that their
             * resource slot preloaded (g_abObjectShapeOwned 0), and loads
             * one for the ship only when none was preloaded.  Retail frees
             * either kind here, so destroying a ship frees a preloaded view
             * under every other ship of its type showing that view.  Free
             * only a packet this ship owns; free_ship releases the rest. */
            if (g_abObjectShapeOwned[obj] != 0)
                FreePacketAndClear(&g_apObjectShape[obj], 0);
            g_abObjectShapeOwned[obj] = 0;
        }
#else
            FreePacketAndClear(&g_apObjectShape[obj], 0);
#endif
        g_asShipSide[obj] = SIDE_NEUTRAL;
        g_acShipPortrait[obj] = -1;
        g_acShipPendingMessage[obj] = -1;
        g_asShipManeuver[obj] = MANEUVER_NONE;
        clear_alert(obj);
        g_asLoadedShipViewFrame[obj] = -1;
        g_asShipIntelSlot[obj] = -1;
        g_asShipIntelResourceKey[obj] = -1;
    }
    g_aeObjectClass[obj] = OBJECT_CLASS_NULL;
    g_apObjectShape[obj] = 0;
}

/* Function start: 0x40C46C */
void apply_force_to_objects_center(FixedVector *force, short obj)
{
    FixedVector acceleration;

    divide_vector(force,
                  g_aObjectTypeData[
                      g_acObjectType[obj]].radarRadius << 8,
                  &acceleration);
    AddFixedVectors(&g_aShipVelocity[obj], &acceleration,
                    &g_aShipVelocity[obj]);
}

/* Function start: 0x40C4D1 */
void apply_force_to_object(FixedVector *point, FixedVector *force,
                           short obj)
{
    FixedVector bodyPoint;
    FixedVector bodyForce;
    FixedVector linearAcceleration;
    int rotationalMass;
    int linearMass;

    transform_to_objects_frame(force, &bodyForce, obj);
    transform_to_objects_frame(point, &bodyPoint, obj);
    linearMass = g_aObjectTypeData[
        g_acObjectType[obj]].radarRadius;
    rotationalMass = DivideFixed(
        g_aObjectTypeData[
            g_acObjectType[obj]].afterburnerVelocity << 8,
        (int)g_asObjectCollisionRadius[obj] << 8);

    g_anObjectRollRotation[obj] += DivideFixed(
        MultiplyFixed(bodyPoint.x, bodyForce.y) -
            MultiplyFixed(bodyPoint.y, bodyForce.x),
        rotationalMass) >> 8;
    g_anObjectYawRotation[obj] += DivideFixed(
        MultiplyFixed(bodyPoint.x, bodyForce.z) -
            MultiplyFixed(bodyPoint.z, bodyForce.x),
        rotationalMass) >> 8;
    g_anObjectPitchRotation[obj] += DivideFixed(
        MultiplyFixed(bodyPoint.z, bodyForce.y) -
            MultiplyFixed(bodyPoint.y, bodyForce.z),
        rotationalMass) >> 8;
    ClampTo30(&g_anObjectPitchRotation[obj]);
    ClampTo30(&g_anObjectYawRotation[obj]);
    ClampTo30(&g_anObjectRollRotation[obj]);

    linearMass <<= 8;
    linearAcceleration.x = DivideFixed(
        MultiplyFixed(0x16a -
            PlanarMagnitude(bodyPoint.y, bodyPoint.z),
            bodyForce.x),
        MultiplyFixed(0x16a, linearMass));
    linearAcceleration.y = DivideFixed(
        MultiplyFixed(0x16a -
            PlanarMagnitude(bodyPoint.x, bodyPoint.z),
            bodyForce.y),
        MultiplyFixed(0x16a, linearMass));
    linearAcceleration.z = DivideFixed(
        MultiplyFixed(0x16a -
            PlanarMagnitude(bodyPoint.x, bodyPoint.y),
            bodyForce.z),
        MultiplyFixed(0x16a, linearMass));
    AddFixedVectors(&g_aShipVelocity[obj], &linearAcceleration,
                    &g_aShipVelocity[obj]);
    if (g_aeObjectClass[obj] == OBJECT_CLASS_SHIP)
        check_for_lost_control(obj);
}

/* Function start: 0x40C796 */
void rotational_acceleration(FixedVector *point, FixedVector *force,
                             short obj)
{
    FixedVector bodyPoint;
    FixedVector bodyForce;
    int denominator;

    transform_to_objects_frame(force, &bodyForce, obj);
    transform_to_objects_frame(point, &bodyPoint, obj);
    denominator = DivideFixed(
        g_aObjectTypeData[
            g_acObjectType[obj]].afterburnerVelocity << 8,
        MultiplyFixed(
            (int)g_asObjectCollisionRadius[obj] << 8,
            0x123c));

    g_anObjectRollRotation[obj] += DivideFixed(
        MultiplyFixed(bodyPoint.x, bodyForce.y) -
            MultiplyFixed(bodyPoint.y, bodyForce.x),
        denominator) >> 8;
    g_anObjectYawRotation[obj] += DivideFixed(
        MultiplyFixed(bodyPoint.x, bodyForce.z) -
            MultiplyFixed(bodyPoint.z, bodyForce.x),
        denominator) >> 8;
    g_anObjectPitchRotation[obj] += DivideFixed(
        MultiplyFixed(bodyPoint.z, bodyForce.y) -
            MultiplyFixed(bodyPoint.y, bodyForce.z),
        denominator) >> 8;
    ClampTo30(&g_anObjectPitchRotation[obj]);
    ClampTo30(&g_anObjectYawRotation[obj]);
    ClampTo30(&g_anObjectRollRotation[obj]);
    if (g_aeObjectClass[obj] == OBJECT_CLASS_SHIP)
        check_for_lost_control(obj);
}

/* Function start: 0x40C959 */
void ClampVectorTo30(short *p)
{
    if (*p < 0) {
        *p = *p + 1;
    } else if (*p > 0) {
        *p = *p - 1;
    }
}

/* Function start: 0x40C99F */
void ClampTo30(short *p)
{
    if (*p > 0x1e) {
        *p = 0x1e;
        return;
    }
    if (*p < -0x1e)
        *p = -0x1e;
}

/* Function start: 0x40C9DD */
void transform_objects_to_your_view(void)
{
    FixedVector direction;
    int range;
    FixedVector viewPos;
    short dustSize;
    short obj;
    int radius;
    short screenZoom;
#ifdef SDL_PORT
    double exactZoom;
#endif

    draw_nav_pointer();
    for (obj = 0; obj <= SPACE_LAST_MOVING_OBJECT; obj++) {
        if (g_aeObjectClass[obj] == OBJECT_CLASS_NULL ||
            g_aeObjectClass[obj] == OBJECT_CLASS_FIXED_OBJECT)
            continue;
        if (obj == g_nNavPointerObject)
            continue;
        {
            g_asObjectDistance[obj] = 0;
            if (g_aeObjectClass[obj] == OBJECT_CLASS_FUTURION) {
                g_asObjectScreenX[obj] = (short)0x8001;
                continue;
            }
            if (obj < 10 && g_anShipCloakState[obj] == 1 &&
                (g_asShipCloakElapsedFrames[obj] >= 20 ||
                 g_anCloakVisibilityPattern[
                     g_asShipCloakElapsedFrames[obj]] == 1)) {
                g_asObjectScreenX[obj] = (short)0x8001;
                continue;
            }
            if (g_aeObjectClass[obj] == OBJECT_CLASS_PLANET ||
                g_aeObjectClass[obj] == OBJECT_CLASS_STAR) {
                direction = g_aShipPosition[obj];
            } else {
                ComputeVectorDelta(&g_aShipPosition[EYE_OBJECT],
                                   &g_aShipPosition[obj],
                                   &direction);
            }
            range = Vector_magnitude(&direction);
            if (range <
                g_asObjectCollisionRadius[EYE_OBJECT] * 0x100) {
                g_asObjectScreenX[obj] = (short)0x8001;
                continue;
            }
            if (g_aeObjectClass[obj] == OBJECT_CLASS_DUST &&
                range > 0x57800) {
                g_asObjectScreenX[obj] = (short)0x8001;
                continue;
            }
            transform_to_objects_frame(&direction, &viewPos,
                                       EYE_OBJECT);
#ifdef SDL_PORT
            /* The enhanced object layer needs the unrounded projection to
             * keep fixed thruster children attached to their ships. */
            g_aObjectViewPosition[obj] = viewPos;
#endif
            if (viewPos.z <
                g_asObjectCollisionRadius[EYE_OBJECT] * 0x100) {
                g_asObjectScreenX[obj] = (short)0x8001;
                continue;
            }
            if (DivideFixed(viewPos.z, range) < 0) {
                g_asObjectScreenX[obj] = (short)0x8001;
                continue;
            }
            radius = g_asObjectCollisionRadius[obj] * 0x100;
            if (range <= radius)
                range = radius + 1;
            if (g_aeObjectClass[obj] >= OBJECT_CLASS_EXPLOSION) {
#ifdef SDL_PORT
                /* Wing2.exe takes the screen-width-to-range ratio with only
                 * eight fraction bits, so a distant object's scale held still
                 * and then jumped: the Concordia's by a third at 11 km and by
                 * a quarter at 9 km (issue #62). WC1 keeps eight more bits,
                 * and DOS WC2 scales smoothly. Take the same scale without
                 * truncating the ratio, in double so a ship close enough to
                 * fill the view cannot overflow. */
                exactZoom = (double)(unsigned short)g_asObjectScale[obj] *
                            (double)(g_nScreenWidth & ~1) * 128.0 /
                            (double)(range -
                                     g_asObjectCollisionRadius[obj] * 0x100);
                screenZoom = (short)(exactZoom < 0x2000 ? exactZoom : 0x2000);
#else
                screenZoom = (short)MultiplyFixed(
                    (unsigned short)g_asObjectScale[obj],
                    DivideFixed(
                        (g_nScreenWidth & ~1) << 7,
                        range -
                            g_asObjectCollisionRadius[obj] * 0x100));
#endif
                screenZoom = MaxShort(0, MinShort(0x2000, screenZoom));
                g_asObjectScreenScale[obj] = screenZoom;
#ifdef SDL_PORT
                /* The enhanced layer draws at the unrounded scale. */
                SdlSetObjectScreenScale(obj, screenZoom,
                                        exactZoom < 0x2000 ? exactZoom
                                                           : 0x2000);
#endif
                if ((unsigned short)g_asObjectScreenScale[obj] <= 4) {
                    g_asObjectScreenX[obj] = (short)0x8001;
                    continue;
                }
            }
            g_asObjectDistance[obj] = (short)(range >> 8);
            if (range > 0xfffe00)
                g_asObjectDistance[obj] = (short)0xfffe;
            g_asObjectScreenX[obj] = (short)DivideFixed(
                MultiplyFixed(g_nScreenWidth >> 1,
                              viewPos.x),
                viewPos.z);
            g_asObjectScreenY[obj] = (short)DivideFixed(
                MultiplyFixed(g_nScreenWidth >> 1,
                              viewPos.y),
                viewPos.z);
            switch (g_aeObjectClass[obj]) {
            case OBJECT_CLASS_STAR:
                break;
            case OBJECT_CLASS_DUST:
                dustSize = (short)(MultiplyFixed(
                    0x900, DivideFixed(
                        g_asObjectCollisionRadius[EYE_OBJECT] << 8,
                        range)) >> 8);
                if (dustSize > 3)
                    dustSize = 3;
                dustSize = (short)(
                    (g_asObjectScreenAngle[obj] & 0x10) +
                    (3 - dustSize) * 4);
                g_asObjectViewFrame[obj] =
                    (short)(((g_asObjectCounter[obj] +
                              g_nSpaceFrame) & 3) +
                            dustSize);
                break;
            case OBJECT_CLASS_PLANET:
                if ((unsigned short)g_asObjectScreenScale[obj] ==
                    0xff)
                    set_background_objects_rotation(obj, &direction);
                break;
            case OBJECT_CLASS_DEBRIS:
            case OBJECT_CLASS_PROJECTILE:
            case OBJECT_CLASS_ASTEROID:
                break;
            case OBJECT_CLASS_EXPLOSION:
                break;
            case OBJECT_CLASS_MISSILE:
            case OBJECT_CLASS_SHIP:
            case OBJECT_CLASS_CAPITAL_SHIP:
            case 14:
                if (g_asObjectType[obj] != 0x14)
                    get_right_shape(obj, &direction);
                break;
            }
        }
    }
}

/* Function start: 0x40CEF6 */
void set_background_objects_rotation(short obj, FixedVector *direction)
{
    SphericalVector spherical;
    FixedVector flatUp;
    short rollAngle;

    negate_vector(direction);
    rectangular_to_spherical(direction, &spherical);
    init_ijk(SCRATCH_VIEW_OBJECT);
    alter_yaw((short)-spherical.yaw, SCRATCH_VIEW_OBJECT);
    alter_pitch((short)-spherical.pitch, SCRATCH_VIEW_OBJECT);
    flatUp.z = 0;
    flatUp.x = dot_product(
        &g_aShipUpVector[EYE_OBJECT],
        &g_aShipRightVector[SCRATCH_VIEW_OBJECT]);
    flatUp.y = dot_product(
        &g_aShipUpVector[EYE_OBJECT],
        &g_aShipUpVector[SCRATCH_VIEW_OBJECT]);
    NormalizeFixedVector(&flatUp);
    rollAngle = (short)ArcCos(flatUp.y);
    rollAngle %= 360;
    if (flatUp.x >= 0)
        rollAngle = 360 - rollAngle;
    g_asObjectScreenAngle[obj] = rollAngle;
    g_asObjectScreenScale[obj] = 0xff;
}

#pragma function(strcat, strcpy, strlen)
/* Function start: 0x40CFF8 */
void get_right_shape(short obj, FixedVector *direction)
{
    SphericalVector angles;
    short yawIndex;
    char fileName[20];
    short shipType;
    short slotIndex;
    FixedVector shipForward;
    FixedVector projectedUpVector;
    short twist;
    short pitchBand;
    FixedVector right;
    char numberString[4];
    short directionIndex;
    short shipClass;
    void **packetGroup;
    FixedVector eyeUp;
    FixedVector up;
    FixedVector forwardAxis;

    right.x = 0x100;
    right.y = 0;
    right.z = 0;
    up.x = 0;
    up.y = 0x100;
    up.z = 0;
    forwardAxis.x = 0;
    forwardAxis.y = 0;
    forwardAxis.z = 0x100;
    negate_vector(direction);
    rectangular_to_spherical(direction, &angles);
    rotate_about_j((short)-angles.yaw, &right, &forwardAxis);
    rotate_about_i((short)-angles.pitch, &up, &forwardAxis);
    NormalizeFixedVector(&up);
    NormalizeFixedVector(&forwardAxis);
    transform_to_objects_frame(&forwardAxis, &shipForward, obj);
    transform_to_objects_frame(&g_aShipUpVector[EYE_OBJECT],
                               &eyeUp, obj);
    rectangular_to_spherical(&shipForward, &angles);
    pitchBand = (short)(angles.pitch / 30 + 3);
    if (angles.pitch % 30 > 15)
        pitchBand = MinShort(6, (short)(pitchBand + 1));
    else if (angles.pitch % 30 < -15)
        pitchBand = MaxShort(0, (short)(pitchBand - 1));
    yawIndex = (short)((12 - angles.yaw / 30) % 12);
    if (angles.yaw % 30 > 15)
        yawIndex = (short)((yawIndex + 11) % 12);
    else if (angles.yaw % 30 < -15)
        yawIndex = (short)((yawIndex + 1) % 12);
    switch (pitchBand) {
    case 0:
        directionIndex = 0;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        directionIndex = (short)((pitchBand - 1) * 12 + yawIndex + 1);
        break;
    case 6:
        directionIndex = 61;
        break;
    }
    projectedUpVector.z = 0;
    projectedUpVector.x = dot_product(
        &eyeUp, &g_9_aDirectionLookRightVector[directionIndex]);
    projectedUpVector.y = dot_product(
        &eyeUp, &g_137_aDirectionViewUpVectorArray[directionIndex]);
    NormalizeFixedVector(&projectedUpVector);
    twist = ArcCos(projectedUpVector.y);
    if (projectedUpVector.x >= 0)
        twist = (short)(360 - twist);
    shipClass = g_aeObjectClass[obj];
    if (shipClass >= OBJECT_CLASS_SHIP) {
        shipType = g_aObjectTypeData[
            g_acObjectType[obj]].resourceType;
    } else {
        shipType = g_asObjectType[obj];
    }
    if (shipClass == OBJECT_CLASS_MISSILE ||
        (shipClass < OBJECT_CLASS_SHIP && shipType == 12)) {
        directionIndex = (short)(directionIndex + DIRECTION_VIEW_COUNT);
    } else if (shipClass == OBJECT_CLASS_BASE) {
        directionIndex =
            (short)(directionIndex + DIRECTION_VIEW_COUNT * 2);
#ifdef SDL_PORT
        /* SO2.exe 0x40B339: Ayer's Rock (resource 48) uses the fourth table
         * section, a view of its own for each direction. The Kilrathi Saga
         * build lacks that section and reads past the table; DOS SO2.EXE
         * has it. */
        if (g_nSdlCampaign == 2 && shipType == 0x30)
            directionIndex =
                (short)(directionIndex + DIRECTION_VIEW_COUNT);
#endif
    }
#ifdef SDL_PORT
    /* SO2.exe 0x40B351: resources 48 and 49 skip the frame 0 and frame 36
     * turns; Ayer's Rock turns its bottom view, frame 61, instead. */
    if (g_acDirectionShapeFrame[directionIndex] == 0 &&
        !(g_nSdlCampaign == 2 && (shipType == 0x30 || shipType == 0x31)))
        twist = (short)(twist + 90);
    if (g_nSdlCampaign == 2 && shipType == 0x30) {
        if (g_acDirectionShapeFrame[directionIndex] == 0x3d)
            twist = (short)(twist + 90);
    } else if (!(g_nSdlCampaign == 2 && shipType == 0x31) &&
               g_acDirectionShapeFrame[directionIndex] == 0x24 &&
               shipClass != OBJECT_CLASS_MISSILE)
        twist = (short)(twist - 90);
#else
    if (g_acDirectionShapeFrame[directionIndex] == 0)
        twist = (short)(twist + 90);
    if (g_acDirectionShapeFrame[directionIndex] == 0x24 &&
        shipClass != OBJECT_CLASS_MISSILE)
        twist = (short)(twist - 90);
#endif
    g_asObjectFlip[obj] =
        (short)(g_acDirectionShapeFlip[directionIndex] << 4);
    twist = (short)(twist % 360);
    if (twist < 0)
        twist = (short)(twist + 360);
    g_asObjectScreenAngle[obj] = twist;
    if (g_asObjectType[obj] == 0x33 ||
        shipClass >= OBJECT_CLASS_CAPITAL_SHIP ||
        (g_bExpandedShipGraphicsEnabled != 0 &&
         shipClass == OBJECT_CLASS_SHIP)) {
        if (g_acDirectionShapeFrame[directionIndex] !=
            g_asLoadedShipViewFrame[obj]) {
            for (slotIndex = 0; slotIndex < 4; slotIndex++) {
                if (g_aObjectResourceSlots[slotIndex].resourceType ==
                    shipType)
                    break;
            }
            g_asObjectViewFrame[obj] = 0;
            g_asLoadedShipViewFrame[obj] =
                g_acDirectionShapeFrame[directionIndex];
            if (g_27_stViewScratchInfo.pixels != 0 &&
                IdentityWord(
                    (unsigned int)g_apObjectShape[obj]) == 0)
                free_view_buffer();
            if (g_abObjectShapeOwned[obj] != 0)
                FreePacketAndClear(&g_apObjectShape[obj], 0);
            packetGroup = g_apPacketReferenceGroups[slotIndex];
            if (packetGroup != 0 &&
                packetGroup[g_asLoadedShipViewFrame[obj]] != 0) {
                g_apObjectShape[obj] =
                    packetGroup[g_asLoadedShipViewFrame[obj]];
                CheckHeapBlockSignature(g_apObjectShape[obj]);
                g_abObjectShapeOwned[obj] = 0;
            } else {
                if (g_27_stViewScratchInfo.pixels != 0)
                    free_view_buffer();
                strcpy(fileName, "ship.v");
                if (g_bExpandedShipGraphicsEnabled != 0 &&
                    shipClass == OBJECT_CLASS_SHIP)
                    strcpy(fileName, "xship.v");
                itoa((int)shipType, numberString, 10);
                if (strlen(numberString) == 1)
                    strcat(fileName, "0");
                strcat(fileName, numberString);
#ifdef SDL_PORT
                /* SO2.exe 0x40B683: Ayer's Rock's views from 37 on follow
                 * the four records in sections 37-40. */
                g_apObjectShape[obj] = FetchDiskPacketRetrying(
                    fileName,
                    (short)(g_asLoadedShipViewFrame[obj] +
                            (g_nSdlCampaign == 2 && shipType == 0x30 &&
                                     g_asLoadedShipViewFrame[obj] >= 0x25
                                 ? 4 : 0)),
                    0);
#else
                g_apObjectShape[obj] = FetchDiskPacketRetrying(
                    fileName, g_asLoadedShipViewFrame[obj], 0);
#endif
                CheckHeapBlockSignature(g_apObjectShape[obj]);
                g_abObjectShapeOwned[obj] = 1;
            }
            initialize_view_buffer();
        }
    } else {
        g_asObjectViewFrame[obj] =
            g_acDirectionShapeFrame[directionIndex];
    }
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
FixedVector g_2_vNormalizedToTgt; /* 0x005d3bd0 */
int g_nUnusedGeomWord[3]; /* 0x005d3be0 */ /* never referenced */
