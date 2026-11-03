/*
 *  Ship velocity limits, vector arithmetic and orientation geometry.
 *
 *  Retail object 0x40A3A0-0x40D6AF (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x40A3A0 */
short get_ship_max_velocity(short obj)
{
    short velocity = g_aObjectTypeData_00496d30[
        g_acObjectType_00493980[obj]].maximumVelocity;

    if (g_asShipSide_004955d0[obj] != g_asShipSide_004955d0[0] &&
        g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP) {
        velocity = (short)(
            (int)MinShort(150, MaxShort(80, GetAdaptiveTurnRate())) *
            velocity / 100);
    }
    if (g_asPilotLevel_00495d60[obj] == RATING_ACE_SPIRIT &&
        g_asShipSide_004955d0[obj] == SIDE_KILRATHI)
        return velocity + velocity / 3;
    else
        return velocity;
}

/* Function start: 0x40A481 */
void recalc_max_velocity(short ship)
{
    short oldVelocity;

    oldVelocity = g_asShipMaximumVelocity_00495f70[ship];
    if (g_anShipFuel_00495638[ship] <= 0) {
        g_asShipMaximumVelocity_00495f70[ship] = 5;
    } else {
        g_asShipMaximumVelocity_00495f70[ship] =
            (short)(((4 - (int)g_acShipIonDriveDamage_004956a0[ship]) *
                     (int)get_ship_max_velocity(ship)) >> 2);
    }
    if (g_asShipMaximumVelocity_00495f70[ship] != oldVelocity)
        celerate(ship, 0);
}

/* Function start: 0x40A51D */
void drain_fuel(short ship, short amount)
{
    g_anShipFuel_00495638[ship] -= (int)amount;
    if (g_anShipFuel_00495638 == 0)
        recalc_max_velocity(ship);
}

/* Function start: 0x40A556 */
void damage_ion_drive(short ship, short amount, short maximum)
{
    g_acShipIonDriveDamage_004956a0[ship] = (signed char)MaxShort(
        0, MinShort((short)(g_acShipIonDriveDamage_004956a0[ship] + amount),
                    maximum));
    recalc_max_velocity(ship);
}

/* Function start: 0x40A5A0 */
int GetShipAccelerationRate(short ship)
{
    int adaptiveScale;
    int acceleration;

    acceleration = g_aObjectTypeData_00496d30[
        g_acObjectType_00493980[ship]].acceleration;
    if (g_asShipSide_004955d0[ship] != g_asShipSide_004955d0[0] &&
        g_aeObjectClass_00495328[ship] == OBJECT_CLASS_SHIP) {
        adaptiveScale =
            (int)MinShort(150, MaxShort(80, GetAdaptiveTurnRate())) << 8;
        acceleration = MultiplyFixed(acceleration, adaptiveScale);
        acceleration = DivideFixed(acceleration, 100 << 8);
    }
    if (g_asPilotLevel_00495d60[ship] == RATING_ACE_SPIRIT &&
        g_asShipSide_004955d0[ship] == SIDE_KILRATHI)
        return acceleration + acceleration / 3;
    else
        return acceleration;
}

/* Function start: 0x40A691 */
void point_at(short obj, FixedVector point)
{
    FixedVector direction;

    ComputeVectorDelta(&g_aShipPosition_00494550[obj], &point, &direction);
    shrink_vector(&direction);
    g_aShipForwardVector_00494208[obj] = direction;
    fix_objects_ijk(obj);
}

/* Function start: 0x40A6F7 */
void look_at(short obj)
{
    point_at(EYE_OBJECT, g_aShipPosition_00494550[obj]);
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
    *position = g_aShipPosition_00494550[obj];
    position_relative(position, g_aShipRightVector_00493b78[obj], right);
    position_relative(position, g_aShipUpVector_00493ec0[obj], up);
    position_relative(position, g_aShipForwardVector_00494208[obj], forward);
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
    g_aShipRightVector_00493b78[obj].x =
        g_aShipUpVector_00493ec0[obj].y =
            g_aShipForwardVector_00494208[obj].z = 0x100;
    g_aShipRightVector_00493b78[obj].y =
        g_aShipRightVector_00493b78[obj].z =
            g_aShipUpVector_00493ec0[obj].x =
                g_aShipUpVector_00493ec0[obj].z =
                    g_aShipForwardVector_00494208[obj].x =
                        g_aShipForwardVector_00494208[obj].y = 0;
    g_anObjectPitchRotation_00494f38[obj] =
        g_anObjectYawRotation_00494fc8[obj] =
            g_anObjectRollRotation_00495058[obj] = 0;
}

/* Function start: 0x40B550 */
void copy_frame(short source, short destination)
{
    g_aShipRightVector_00493b78[destination] =
        g_aShipRightVector_00493b78[source];
    g_aShipUpVector_00493ec0[destination] =
        g_aShipUpVector_00493ec0[source];
    g_aShipForwardVector_00494208[destination] =
        g_aShipForwardVector_00494208[source];
}

/* Function start: 0x40B5DF */
void fix_objects_ijk(short obj)
{
    vector_cross_product(&g_aShipUpVector_00493ec0[obj],
                         &g_aShipForwardVector_00494208[obj],
                         &g_aShipRightVector_00493b78[obj]);
    vector_cross_product(&g_aShipForwardVector_00494208[obj],
                         &g_aShipRightVector_00493b78[obj],
                         &g_aShipUpVector_00493ec0[obj]);
    NormalizeFixedVector(&g_aShipRightVector_00493b78[obj]);
    NormalizeFixedVector(&g_aShipUpVector_00493ec0[obj]);
    NormalizeFixedVector(&g_aShipForwardVector_00494208[obj]);
}

/* Function start: 0x40B699 */
void transform_to_objects_frame(const FixedVector *source,
                                FixedVector *destination, short obj)
{
    destination->x = dot_product(source, &g_aShipRightVector_00493b78[obj]);
    destination->y = dot_product(source, &g_aShipUpVector_00493ec0[obj]);
    destination->z = dot_product(source, &g_aShipForwardVector_00494208[obj]);
}

/* Function start: 0x40B706 */
void alter_pitch(short angle, short obj)
{
    rotate_about_i(angle, &g_aShipUpVector_00493ec0[obj],
                   &g_aShipForwardVector_00494208[obj]);
    fix_objects_ijk(obj);
}

/* Function start: 0x40B747 */
void alter_yaw(short angle, short obj)
{
    rotate_about_j(angle, &g_aShipRightVector_00493b78[obj],
                   &g_aShipForwardVector_00494208[obj]);
    fix_objects_ijk(obj);
}

/* Function start: 0x40B788 */
void alter_roll(short angle, short obj)
{
    rotate_about_k(angle, &g_aShipRightVector_00493b78[obj],
                   &g_aShipUpVector_00493ec0[obj]);
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
    ComputeVectorDelta(&g_aShipPosition_00494550[obj],
                       (FixedVector *)point, &g_vToTarget_00493188);
    return FixedToShortSaturating((int)Vector_magnitude(&g_vToTarget_00493188)) -
           g_asObjectCollisionRadius_004950e8[obj];
}

/* Function start: 0x40B85C */
short distance_from_object(short obj, short other)
{
    return distance_from_point(obj, &g_aShipPosition_00494550[other]) -
           g_asObjectCollisionRadius_004950e8[other];
}

/* Counts facing and range queries; nothing reads it. */
int g_nFacingRangeQueryCount_00492708;

/* Function start: 0x40B898 */
void get_facing_range_from_point(short obj, const FixedVector *point)
{
    int cosine;

    g_nTargetRange_0049319c =
        distance_from_point(obj, point) -
        g_asObjectCollisionRadius_004950e8[obj];
    g_vNormalizedToTarget_005d3bd0 = g_vToTarget_00493188;
    NormalizeFixedVector(&g_vNormalizedToTarget_005d3bd0);
    cosine = dot_product(&g_vNormalizedToTarget_005d3bd0,
                         &g_aShipForwardVector_00494208[obj]);
    g_nFacingToTarget_00493194 = (short)(((short)cosine * 100) >> 8);
    g_nFacingRangeQueryCount_00492708++;
}

/* Function start: 0x40B92F */
void get_facing_range_from_object(short obj, short other)
{
    int cosine;

    get_facing_range_from_point(obj, &g_aShipPosition_00494550[other]);
    g_nTargetRange_0049319c -= g_asObjectCollisionRadius_004950e8[other];
    negate_vector(&g_vNormalizedToTarget_005d3bd0);
    cosine = dot_product(&g_vNormalizedToTarget_005d3bd0,
                         &g_aShipForwardVector_00494208[other]);
    g_nTargetFacing_00493198 = (short)(((short)cosine * 100) >> 8);
}

/* Function start: 0x40B9B5 */
short facing_to_object(short obj, FixedVector *point)
{
    FixedVector direction;
    int cosine;

    ComputeVectorDelta(&g_aShipPosition_00494550[obj], point, &direction);
    NormalizeFixedVector(&direction);
    cosine = dot_product(&direction, &g_aShipForwardVector_00494208[obj]);
    g_nFacingToTarget_00493194 = (short)(((short)cosine * 100) >> 8);
    return g_nFacingToTarget_00493194;
}

/* Function start: 0x40BA2E */
short match_roll_orientation(short obj, short reference)
{
    FixedVector roll;
    short angle;

    roll.x = dot_product(&g_aShipUpVector_00493ec0[obj],
                         &g_aShipRightVector_00493b78[reference]);
    roll.y = dot_product(&g_aShipUpVector_00493ec0[obj],
                         &g_aShipUpVector_00493ec0[reference]);
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
                            &g_anYawGoal_004954c0[obj],
                            &g_anPitchGoal_004954a8[obj]);
    return;
}

/* Function start: 0x40BCA9 */
void point_ship_at_point(short obj, const FixedVector *point)
{
    FixedVector direction;

    ComputeVectorDelta(&g_aShipPosition_00494550[obj],
                       (FixedVector *)point, &direction);
    point_ship(obj, 0, &direction);
}

/* Function start: 0x40BCE8 */
void point_ship_at_object(short obj, short other)
{
    point_ship_at_point(obj, &g_aShipPosition_00494550[other]);
}

/* Function start: 0x40BD0E */
void point_capital_ship_at_object(short obj, short other)
{
    FixedVector direction;

    ComputeVectorDelta(&g_aShipPosition_00494550[other],
                       &g_aShipPosition_00494550[obj], &direction);
    point_ship(obj, 0, &direction);
}

/* Function start: 0x40BD58 */
void point_perpendicular_to_point(short obj, const FixedVector *point)
{
    point_ship_at_point(obj, point);
    if (g_anYawGoal_004954c0[obj] < 0)
        g_anYawGoal_004954c0[obj] += 90;
    else
        g_anYawGoal_004954c0[obj] -= 90;
}

/* Function start: 0x40BDC2 */
void point_ship_behind_object(short obj, short other)
{
    FixedVector point = g_aShipPosition_00494550[other];

    position_relative(&point, g_aShipForwardVector_00494208[other],
                      (short)-(g_asObjectCollisionRadius_004950e8[other] +
                               500));
    point_ship_at_point(obj, &point);
}

/* Function start: 0x40BE3A */
void point_ship_below_object(short obj, short other)
{
    FixedVector point = g_aShipPosition_00494550[other];

    position_relative(&point, g_aShipUpVector_00493ec0[other],
                      (short)(g_asObjectCollisionRadius_004950e8[other] +
                              500));
    point_ship_at_point(obj, &point);
}

/* Function start: 0x40BEB0 */
void point_perpendicular(short obj, short other)
{
    point_perpendicular_to_point(obj, &g_aShipPosition_00494550[other]);
}

/* Function start: 0x40BED6 */
void point_parallel(short obj, short other)
{
    if (other != -1)
        point_ship(obj, 0, &g_aShipForwardVector_00494208[other]);
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

    objectPosition = &g_aShipPosition_00494550[obj];
    position = g_aShipPosition_00494550;
    for (candidate = 0; candidate <= SPACE_LAST_MOVING_OBJECT;
         candidate++, position++) {
        if (candidate != obj &&
            g_aeObjectClass_00495328[candidate] >= OBJECT_CLASS_PROJECTILE) {
            ComputeVectorDelta(objectPosition, position,
                               &g_vCollisionDelta_00493178);
            range = (short)(
#ifdef SDL_PORT
                SdlGetBalancedCollisionRadius(candidate) +
                SdlGetBalancedCollisionRadius(obj));
#else
                g_asObjectCollisionRadius_004950e8[candidate] +
                g_asObjectCollisionRadius_004950e8[obj]);
#endif
            if (g_aeObjectClass_00495328[candidate] == OBJECT_CLASS_SHIP &&
                g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP)
                range >>= 1;
            if (IsVectorWithinRange(&g_vCollisionDelta_00493178,
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

    mount = &((struct ShipLoadout *)g_aShipWeapons_004956b0)[parent].slots[
        weapon].mount;
    position->x = g_aShipUpVector_00493ec0[parent].x * mount->y +
                  g_aShipForwardVector_00494208[parent].x * mount->z +
                  g_aShipRightVector_00493b78[parent].x * mount->x +
                  g_aShipPosition_00494550[parent].x;
    position->y = g_aShipUpVector_00493ec0[parent].y * mount->y +
                  g_aShipRightVector_00493b78[parent].y * mount->x +
                  g_aShipForwardVector_00494208[parent].y * mount->z +
                  g_aShipPosition_00494550[parent].y;
    position->z = g_aShipUpVector_00493ec0[parent].z * mount->y +
                  g_aShipForwardVector_00494208[parent].z * mount->z +
                  g_aShipRightVector_00493b78[parent].z * mount->x +
                  g_aShipPosition_00494550[parent].z;
}

/* Function start: 0x40C1DA */
void child_object(short hardpoint, short child, short parent)
{
    position_child(parent, hardpoint, &g_aShipPosition_00494550[child]);
    g_acObjectOwner_00495208[child] = (signed char)parent;
}

/* Function start: 0x40C211 */
short get_ship_slot(void)
{
    short slot;

    for (slot = 1; slot <= 9; slot++) {
        if (g_aeObjectClass_00495328[slot] == OBJECT_CLASS_NULL)
            return slot;
    }
    return -1;
}

/* Function start: 0x40C266 */
short find_vacant_3d_object(void)
{
    short object;

    for (object = 10; object <= SPACE_LAST_MOVING_OBJECT; object++) {
        if (g_aeObjectClass_00495328[object] == OBJECT_CLASS_NULL) {
            g_asObjectScreenX_00493598[object] = (short)0x8001;
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
    g_asObjectScreenX_00493598[obj] = (short)0x8001;
    g_asObjectDistance_00493ae8[obj] = 0;
    if (obj == g_nNavPointerObject_004931b8)
        g_nNavPointerObject_004931b8 = -1;
    if (obj == g_nYourWingman_0049346c)
        g_nYourWingman_0049346c = -1;
    for (slot = 0; slot < 20; slot++) {
        if (g_abHazardObjects_00493280[slot] == obj) {
            g_abHazardObjects_00493280[slot] = -1;
            break;
        }
    }
    if (obj < 10) {
        if (g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_CAPITAL_SHIP ||
            (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP &&
             g_asObjectType_00495298[obj] == 0x33))
            FreePacketAndClear(&g_apObjectShape_00493868[obj], 0);
        g_asShipSide_004955d0[obj] = SIDE_NEUTRAL;
        g_acShipPortrait_00495d88[obj] = -1;
        g_acShipPendingMessage_00495d98[obj] = -1;
        g_asShipManeuver_00495f48[obj] = MANEUVER_NONE;
        clear_alert(obj);
        g_asLoadedShipViewFrame_00495d18[obj] = -1;
        g_asShipIntelSlot_00495d30[obj] = -1;
        g_asShipIntelResourceKey_00495d48[obj] = -1;
    }
    g_aeObjectClass_00495328[obj] = OBJECT_CLASS_NULL;
    g_apObjectShape_00493868[obj] = 0;
}

/* Function start: 0x40C46C */
void apply_force_to_objects_center(FixedVector *force, short obj)
{
    FixedVector acceleration;

    divide_vector(force,
                  g_aObjectTypeData_00496d30[
                      g_acObjectType_00493980[obj]].radarRadius << 8,
                  &acceleration);
    AddFixedVectors(&g_aShipVelocity_00494898[obj], &acceleration,
                    &g_aShipVelocity_00494898[obj]);
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
    linearMass = g_aObjectTypeData_00496d30[
        g_acObjectType_00493980[obj]].radarRadius;
    rotationalMass = DivideFixed(
        g_aObjectTypeData_00496d30[
            g_acObjectType_00493980[obj]].afterburnerVelocity << 8,
        (int)g_asObjectCollisionRadius_004950e8[obj] << 8);

    g_anObjectRollRotation_00495058[obj] += DivideFixed(
        MultiplyFixed(bodyPoint.x, bodyForce.y) -
            MultiplyFixed(bodyPoint.y, bodyForce.x),
        rotationalMass) >> 8;
    g_anObjectYawRotation_00494fc8[obj] += DivideFixed(
        MultiplyFixed(bodyPoint.x, bodyForce.z) -
            MultiplyFixed(bodyPoint.z, bodyForce.x),
        rotationalMass) >> 8;
    g_anObjectPitchRotation_00494f38[obj] += DivideFixed(
        MultiplyFixed(bodyPoint.z, bodyForce.y) -
            MultiplyFixed(bodyPoint.y, bodyForce.z),
        rotationalMass) >> 8;
    ClampTo30(&g_anObjectPitchRotation_00494f38[obj]);
    ClampTo30(&g_anObjectYawRotation_00494fc8[obj]);
    ClampTo30(&g_anObjectRollRotation_00495058[obj]);

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
    AddFixedVectors(&g_aShipVelocity_00494898[obj], &linearAcceleration,
                    &g_aShipVelocity_00494898[obj]);
    if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP)
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
        g_aObjectTypeData_00496d30[
            g_acObjectType_00493980[obj]].afterburnerVelocity << 8,
        MultiplyFixed(
            (int)g_asObjectCollisionRadius_004950e8[obj] << 8,
            0x123c));

    g_anObjectRollRotation_00495058[obj] += DivideFixed(
        MultiplyFixed(bodyPoint.x, bodyForce.y) -
            MultiplyFixed(bodyPoint.y, bodyForce.x),
        denominator) >> 8;
    g_anObjectYawRotation_00494fc8[obj] += DivideFixed(
        MultiplyFixed(bodyPoint.x, bodyForce.z) -
            MultiplyFixed(bodyPoint.z, bodyForce.x),
        denominator) >> 8;
    g_anObjectPitchRotation_00494f38[obj] += DivideFixed(
        MultiplyFixed(bodyPoint.z, bodyForce.y) -
            MultiplyFixed(bodyPoint.y, bodyForce.z),
        denominator) >> 8;
    ClampTo30(&g_anObjectPitchRotation_00494f38[obj]);
    ClampTo30(&g_anObjectYawRotation_00494fc8[obj]);
    ClampTo30(&g_anObjectRollRotation_00495058[obj]);
    if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP)
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
    short screenScale;
    short dustSize;
    short obj;
    int objectRadius;
    int distance;
    FixedVector viewPosition;
    FixedVector direction;

    draw_nav_pointer();
    for (obj = 0; obj <= SPACE_LAST_MOVING_OBJECT; obj++) {
        if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_NULL ||
            g_aeObjectClass_00495328[obj] == OBJECT_CLASS_FIXED_OBJECT)
            continue;
        if (obj == g_nNavPointerObject_004931b8)
            continue;
        {
            g_asObjectDistance_00493ae8[obj] = 0;
            if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_FUTURION) {
                g_asObjectScreenX_00493598[obj] = (short)0x8001;
                continue;
            }
            if (obj < 10 && g_anShipCloakState_00496020[obj] == 1 &&
                (g_asShipCloakElapsedFrames_00496060[obj] >= 20 ||
                 g_anCloakVisibilityPattern_00492710[
                     g_asShipCloakElapsedFrames_00496060[obj]] == 1)) {
                g_asObjectScreenX_00493598[obj] = (short)0x8001;
                continue;
            }
            if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_PLANET ||
                g_aeObjectClass_00495328[obj] == OBJECT_CLASS_STAR) {
                direction = g_aShipPosition_00494550[obj];
            } else {
                ComputeVectorDelta(&g_aShipPosition_00494550[EYE_OBJECT],
                                   &g_aShipPosition_00494550[obj],
                                   &direction);
            }
            distance = Vector_magnitude(&direction);
            if (distance <
                g_asObjectCollisionRadius_004950e8[EYE_OBJECT] * 0x100) {
                g_asObjectScreenX_00493598[obj] = (short)0x8001;
                continue;
            }
            if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_DUST &&
                distance > 0x57800) {
                g_asObjectScreenX_00493598[obj] = (short)0x8001;
                continue;
            }
            transform_to_objects_frame(&direction, &viewPosition,
                                       EYE_OBJECT);
#ifdef SDL_PORT
            /* The enhanced object layer needs the unrounded projection to
             * keep fixed thruster children attached to their ships. */
            g_aObjectViewPosition_0059afa0[obj] = viewPosition;
#endif
            if (viewPosition.z <
                g_asObjectCollisionRadius_004950e8[EYE_OBJECT] * 0x100) {
                g_asObjectScreenX_00493598[obj] = (short)0x8001;
                continue;
            }
            if (DivideFixed(viewPosition.z, distance) < 0) {
                g_asObjectScreenX_00493598[obj] = (short)0x8001;
                continue;
            }
            objectRadius = g_asObjectCollisionRadius_004950e8[obj] * 0x100;
            if (distance <= objectRadius)
                distance = objectRadius + 1;
            if (g_aeObjectClass_00495328[obj] > OBJECT_CLASS_DUST) {
                screenScale = (short)MultiplyFixed(
                    (unsigned short)g_asObjectScale_00494d90[obj],
                    DivideFixed(
                        (g_nScreenWidth_0049d4d8 & ~1) << 7,
                        distance - objectRadius));
                screenScale = MinShort(0x2000, screenScale);
                screenScale = MaxShort(0, screenScale);
                g_asObjectScreenScale_00493a58[obj] = screenScale;
                if ((unsigned short)g_asObjectScreenScale_00493a58[obj] <= 4) {
                    g_asObjectScreenX_00493598[obj] = (short)0x8001;
                    continue;
                }
            }
            g_asObjectDistance_00493ae8[obj] = (short)(distance >> 8);
            if (distance > 0xfffe00)
                g_asObjectDistance_00493ae8[obj] = (short)0xfffe;
            g_asObjectScreenX_00493598[obj] = (short)DivideFixed(
                MultiplyFixed(g_nScreenWidth_0049d4d8 >> 1,
                              viewPosition.x),
                viewPosition.z);
            g_asObjectScreenY_00493628[obj] = (short)DivideFixed(
                MultiplyFixed(g_nScreenWidth_0049d4d8 >> 1,
                              viewPosition.y),
                viewPosition.z);
            switch (g_aeObjectClass_00495328[obj]) {
            case OBJECT_CLASS_STAR:
                break;
            case OBJECT_CLASS_DUST:
                dustSize = (short)(MultiplyFixed(
                    0x900, DivideFixed(
                        g_asObjectCollisionRadius_004950e8[EYE_OBJECT] << 8,
                        distance)) >> 8);
                if (dustSize > 3)
                    dustSize = 3;
                dustSize = (short)(
                    (g_asObjectScreenAngle_004936b8[obj] & 0x10) +
                    (3 - dustSize) * 4);
                g_asObjectViewFrame_00493508[obj] =
                    (short)(((g_asObjectCounter_00494be0[obj] +
                              g_nSpaceFrame_00493134) & 3) +
                            dustSize);
                break;
            case OBJECT_CLASS_PLANET:
                if ((unsigned short)g_asObjectScreenScale_00493a58[obj] ==
                    0xff)
                    set_background_objects_rotation(obj, &direction);
                break;
            case OBJECT_CLASS_EXPLOSION:
            case OBJECT_CLASS_DEBRIS:
            case OBJECT_CLASS_PROJECTILE:
            case OBJECT_CLASS_ASTEROID:
            case OBJECT_CLASS_MINE:
                break;
            case OBJECT_CLASS_MISSILE:
            case OBJECT_CLASS_SHIP:
            case OBJECT_CLASS_CAPITAL_SHIP:
            case 14:
                if (g_asObjectType_00495298[obj] != 0x14)
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
        &g_aShipUpVector_00493ec0[EYE_OBJECT],
        &g_aShipRightVector_00493b78[SCRATCH_VIEW_OBJECT]);
    flatUp.y = dot_product(
        &g_aShipUpVector_00493ec0[EYE_OBJECT],
        &g_aShipUpVector_00493ec0[SCRATCH_VIEW_OBJECT]);
    NormalizeFixedVector(&flatUp);
    rollAngle = (short)ArcCos(flatUp.y);
    rollAngle %= 360;
    if (flatUp.x >= 0)
        rollAngle = 360 - rollAngle;
    g_asObjectScreenAngle_004936b8[obj] = rollAngle;
    g_asObjectScreenScale_00493a58[obj] = 0xff;
}

#pragma function(strcat)
/* Function start: 0x40CFF8 */
void get_right_shape(short obj, FixedVector *direction)
{
    short type;
    char fileName[12];
    short yawSector;
    SphericalVector spherical;
    short slot;
    short angle;
    FixedVector projectedUp;
    FixedVector objectForward;
    short pitchBand;
    short directionIndex;
    char numberText[12];
    FixedVector right;
    void **resourceGroup;
    short objectClass;
    FixedVector eyeUp;
    FixedVector up;
    FixedVector forward;

    right.x = 0x100;
    right.y = 0;
    right.z = 0;
    up.x = 0;
    up.y = 0x100;
    up.z = 0;
    forward.x = 0;
    forward.y = 0;
    forward.z = 0x100;
    negate_vector(direction);
    rectangular_to_spherical(direction, &spherical);
    rotate_about_j((short)-spherical.yaw, &right, &forward);
    rotate_about_i((short)-spherical.pitch, &up, &forward);
    NormalizeFixedVector(&up);
    NormalizeFixedVector(&forward);
    transform_to_objects_frame(&forward, &objectForward, obj);
    transform_to_objects_frame(&g_aShipUpVector_00493ec0[EYE_OBJECT],
                               &eyeUp, obj);
    rectangular_to_spherical(&objectForward, &spherical);
    pitchBand = (short)(spherical.pitch / 30 + 3);
    if (spherical.pitch % 30 > 15)
        pitchBand = MinShort(6, (short)(pitchBand + 1));
    else if (spherical.pitch % 30 < -15)
        pitchBand = MaxShort(0, (short)(pitchBand - 1));
    yawSector = (short)((12 - spherical.yaw / 30) % 12);
    if (spherical.yaw % 30 > 15)
        yawSector = (short)((yawSector + 11) % 12);
    else if (spherical.yaw % 30 < -15)
        yawSector = (short)((yawSector + 1) % 12);
    switch (pitchBand) {
    case 0:
        directionIndex = 0;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        directionIndex = (short)(pitchBand * 12 + yawSector - 11);
        break;
    case 6:
        directionIndex = 61;
        break;
    }
    projectedUp.z = 0;
    projectedUp.x = dot_product(
        &eyeUp, &g_aDirectionViewRightVector_005d2210[directionIndex]);
    projectedUp.y = dot_product(
        &eyeUp, &g_aDirectionViewUpVector_005d2500[directionIndex]);
    NormalizeFixedVector(&projectedUp);
    angle = ArcCos(projectedUp.y);
    if (projectedUp.x >= 0)
        angle = (short)(360 - angle);
    objectClass = g_aeObjectClass_00495328[obj];
    if (objectClass >= OBJECT_CLASS_SHIP) {
        type = g_aObjectTypeData_00496d30[
            g_acObjectType_00493980[obj]].resourceType;
    } else {
        type = g_asObjectType_00495298[obj];
    }
    if (objectClass == OBJECT_CLASS_MISSILE ||
        (objectClass < OBJECT_CLASS_SHIP && type == 12)) {
        directionIndex = (short)(directionIndex + DIRECTION_VIEW_COUNT);
    } else if (objectClass == OBJECT_CLASS_BASE) {
        directionIndex =
            (short)(directionIndex + DIRECTION_VIEW_COUNT * 2);
    }
    if (g_acDirectionShapeFrame_0049d558[directionIndex] == 0)
        angle = (short)(angle + 90);
    if (g_acDirectionShapeFrame_0049d558[directionIndex] == 0x24 &&
        objectClass != OBJECT_CLASS_MISSILE)
        angle = (short)(angle - 90);
    g_asObjectFlip_004939c8[obj] =
        (short)(g_acDirectionShapeFlip_0049d618[directionIndex] << 4);
    angle = (short)(angle % 360);
    if (angle < 0)
        angle = (short)(angle + 360);
    g_asObjectScreenAngle_004936b8[obj] = angle;
    if (g_asObjectType_00495298[obj] == 0x33 ||
        objectClass >= OBJECT_CLASS_CAPITAL_SHIP ||
        (g_bExpandedShipGraphicsEnabled_004931a4 != 0 &&
         objectClass == OBJECT_CLASS_SHIP)) {
        if (g_acDirectionShapeFrame_0049d558[directionIndex] !=
            g_asLoadedShipViewFrame_00495d18[obj]) {
            for (slot = 0; slot < 4; slot++) {
                if (g_aObjectResourceSlots_00493398[slot].resourceType ==
                    type)
                    break;
            }
            g_asObjectViewFrame_00493508[obj] = 0;
            g_asLoadedShipViewFrame_00495d18[obj] =
                g_acDirectionShapeFrame_0049d558[directionIndex];
            if (g_stViewBuffer_005d2b00.pixels != 0 &&
                IdentityWord(
                    (unsigned int)g_apObjectShape_00493868[obj]) == 0)
                free_view_buffer();
            if (g_abObjectShapeOwned_00492760[obj] != 0)
                FreePacketAndClear(&g_apObjectShape_00493868[obj], 0);
            resourceGroup = g_apPacketReferenceGroups_0049b898[slot];
            if (resourceGroup != 0 &&
                resourceGroup[g_asLoadedShipViewFrame_00495d18[obj]] != 0) {
                g_apObjectShape_00493868[obj] =
                    resourceGroup[g_asLoadedShipViewFrame_00495d18[obj]];
                CheckHeapBlockSignature(g_apObjectShape_00493868[obj]);
                g_abObjectShapeOwned_00492760[obj] = 0;
            } else {
                if (g_stViewBuffer_005d2b00.pixels != 0)
                    free_view_buffer();
                strcpy(fileName, "ship.v");
                if (g_bExpandedShipGraphicsEnabled_004931a4 != 0 &&
                    objectClass == OBJECT_CLASS_SHIP)
                    strcpy(fileName, "xship.v");
                _itoa((int)type, numberText, 10);
                if (strlen(numberText) == 1)
                    strcat(fileName, "0");
                strcat(fileName, numberText);
                g_apObjectShape_00493868[obj] = FetchDiskPacketRetrying(
                    fileName, g_asLoadedShipViewFrame_00495d18[obj], 0);
                CheckHeapBlockSignature(g_apObjectShape_00493868[obj]);
                g_abObjectShapeOwned_00492760[obj] = 1;
            }
            initialize_view_buffer();
        }
    } else {
        g_asObjectViewFrame_00493508[obj] =
            g_acDirectionShapeFrame_0049d558[directionIndex];
    }
}
