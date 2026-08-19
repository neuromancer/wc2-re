/*
 *  Vector, angle and fixed-point geometry helpers.
 *
 *  Address range 0x418000-0x41ad4f (provisional -- see docs/ORDER.md).
 *  Boundary evidence: geometry helpers followed by the contiguous modal-panel
 *  drawing block, before the save/load tranche at 0x41ada0.
 */
#include "wc1.h"

#pragma function(strcat, memset)

/* Function start: 0x453B30 */
short MeasureTextPixelWidthClamped(const char *text)
{
#if 0
    short width = 0;
    const char *scan = text;

    while (*scan != 0) {
        width = (short)(width + GetFontCharWidth(*scan++));
        if (width >= 320)
            break;
    }
    if (*scan-- != 0) {
        width = (short)(width - GetFontCharWidth(*scan));
    }
    return width;
#else
    const char *scan;
    short width;

    width = 0;
    scan = text;
    while (width < 320 && *scan != 0)
        width = (short)(width + GetFontCharWidth(*scan++));
    if (*scan-- != 0)
        width = (short)(width - GetFontCharWidth(*scan));
    return width;
#endif
}

/* Function start: WC2_UNMAPPED */
int __stdcall SeekPacketSection(PacketSectionHandle *handle, int offset,
                                short origin)
{
    int position;
    int sectionEnd;
    int result;

    sectionEnd = (int)(handle->dataOffset + handle->dataSize);
    switch (origin) {
    case 0:
        position = (int)handle->dataOffset;
        break;
    case 1:
        position = (int)(handle->dataOffset + handle->position);
        break;
    case 2:
        position = sectionEnd;
        break;
    }
    position += offset;
    if (position < (int)handle->dataOffset)
        position = (int)handle->dataOffset;
    if (position > sectionEnd && handle->finalSection == 0)
        position = sectionEnd;
    result = SeekDataFile((unsigned short)handle->file, position, 0);
    if (result != -1)
        handle->position = (unsigned int)(result - handle->dataOffset);
}

/* Function start: WC2_UNMAPPED */
unsigned short GetMusicDriverPresent(short mode)
{
    return 1;
}

/* Function start: 0x4589D0 */
short CollectActivePaletteIndices(Viewport *viewport,
                                  unsigned char *indices,
                                  short capacity)
{
    short count;
    short index;
    unsigned char *active;

    count = 0;
    active = AllocateTaggedMemory((unsigned int)capacity, 0);
    if (active == 0)
        return 0;

    memset(active, 0, (unsigned int)capacity);
    MarkActivePaletteEntries(viewport, active);
    for (index = 0; index < capacity; index++) {
        if (active[index] != 0) {
            indices[count] = (unsigned char)index;
            count++;
        }
    }
    ReleasePacketHandle(active);
    return count;
}

/* Function start: 0x40A3A0 */
short get_ship_max_velocity(short obj)
{
    short velocity = g_aObjectTypeData_00496d30[
        g_acObjectType_00493980[obj]].maximumVelocity;

#if 0
    if (obj < 10 && g_acShipRating_0059cd80[obj] > 8)
        return velocity + velocity / 3;
#else
    if (g_asShipSide_004955d0[obj] != g_asShipSide_004955d0[0] &&
        g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP) {
        velocity = (short)(
            (int)MinShort(150, MaxShort(80, GetAdaptiveTurnRate())) *
            velocity / 100);
    }
    if (g_asPilotLevel_00495d60[obj] == RATING_ACE_SPIRIT &&
        g_asShipSide_004955d0[obj] == SIDE_KILRATHI)
        return velocity + velocity / 3;
#endif
    return velocity;
}

/* Function start: 0x40A481 */
void recalc_max_velocity(short ship)
{
    short oldVelocity;

    oldVelocity = g_asShipMaximumVelocity_00495f70[ship];
    if (g_anShipFuel_00495638[ship] < 1) {
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
#if 0
    volatile signed char *ionDriveDamage;
    int damage;

    ionDriveDamage = &g_acShipIonDriveDamage_004956a0[ship];
    damage = (int)*ionDriveDamage;
    damage += amount;
    if (damage >= maximum)
        damage = maximum;
    if (damage <= 0)
        damage = 0;
    *ionDriveDamage = (signed char)damage;
    recalc_max_velocity(ship);
#else
    g_acShipIonDriveDamage_004956a0[ship] = (signed char)MaxShort(
        0, MinShort((short)(g_acShipIonDriveDamage_004956a0[ship] + amount),
                    maximum));
    recalc_max_velocity(ship);
#endif
}

/* Function start: 0x40A5A0 */
int GetShipAccelerationRate(short ship)
{
#if 0
    int shipIndex = (int)ship;
    enum ObjectType objectType = g_acObjectType_00493980[shipIndex];
    int acceleration = g_aObjectTypeData_00496d30[objectType].acceleration;

    if (ship < 10 && g_acShipRating_0059cd80[shipIndex] > RATING_ACE_ICEMAN)
        return acceleration + acceleration / 3;
    return acceleration;
#else
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
    return acceleration;
#endif
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
    point_at(WC2_EYE_OBJECT, g_aShipPosition_00494550[obj]);
}

/* Function start: 0x40A725 */
void position_relative(FixedVector *position, FixedVector direction,
                       short distance)
{
    if (distance != 0) {
        NormalizeFixedVector(&direction);
#ifdef WC1_SDL
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
    if (value > 0x7fff00)
        return 0x7fff;
    return (short)(value >> 8);
}

/* Function start: 0x40A867 */
int MinInt(int a, int b)
{
    if (a <= b) b = a;
    return b;
}

/* Function start: 0x40A893 */
int MaxInt(int a, int b)
{
    if (b <= a) b = a;
    return b;
}

/* Function start: 0x40A8BF */
int AbsInt(int v)
{
    if (v < 0)
        v = -v;
    return v;
}

/* Function start: WC2_UNMAPPED */
/* Exact Mac symbol: intfract_sign. No inbound reference is known in the
 * shipped executable; this routine is believed unreachable. */
int intfract_sign(int sign, int magnitude)
{
    int result;

    result = magnitude;
    if (sign >= 0)
        return result;
    return -result;
}

/* Function start: WC2_UNMAPPED */
unsigned short SignShort(short v)
{
    if (v < 0)
        return 0xffff;
    return (unsigned short)(0 < v);
}

/* Function start: 0x40A8EB */
unsigned int SignFixed(int v)
{
    if (v < 0)
        return 0xffffff00;
    return v > 0 ? 0x100 : 0;
}

/* Function start: 0x40A925 */
/* Normalise a degree value into (-180, 180]. 0x168 == 360, 0xB4 == 180. */
short WrapDegrees(short degrees)
{
    short v = degrees % 0x168;

    if (v < -0xb4)
        v = v + 0x168;
    if (0xb4 < v)
        v = v - 0x168;
    return v;
}

/* Function start: 0x40A981 */
short equ_vector(const FixedVector *left, const FixedVector *right)
{
    return left->x == right->x && left->y == right->y &&
           left->z == right->z;
}

/* Function start: 0x40A9D1 */
int IsPairEqualityDifferentFromFlag(const unsigned int *values)
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
                                  int allowNegative)
{
    short value = RandomInRange(minimum, maximum);

    if (allowNegative != 0 && RandomInRange(0, 1) != 0)
        value = -value;
    return value;
}

/* Function start: 0x40AC24 */
void MakeRandomVectorFixed(short minimum, short maximum, FixedVector *vector)
{
#ifdef WC1_SDL
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

/* Function start: WC2_UNMAPPED */
void MakeRandomNormalizedVector(FixedVector *vector)
{
    vector->x = (unsigned short)RandomInRange(0x40, 0xff);
    vector->y = (unsigned short)RandomInRange(0x40, 0xff);
    vector->z = (unsigned short)RandomInRange(0x40, 0xff);
    NormalizeFixedVector(vector);
}

/* Function start: 0x40ACE0 */
void rectangular_to_spherical(const FixedVector *rectangular,
                              SphericalVector *spherical)
{
    int horizontalLength;
    int z;

    spherical->radius = Vector_magnitude(rectangular);
    if (spherical->radius == 0)
        return;
    z = rectangular->z;
    horizontalLength = PlanarMagnitude(rectangular->x, z);
    spherical->yaw = (short)ArcCos(
        DivideFixed(z, horizontalLength));
    if (rectangular->x < 0)
        spherical->yaw = -spherical->yaw;
    spherical->pitch = (short)(ArcCos(
        DivideFixed(rectangular->y, spherical->radius)) - 90);
}

/* Function start: WC2_UNMAPPED */
void ConvertShortVectorToFixedVector(const ShortVector *source,
                                     FixedVector *destination)
{
#ifdef WC1_SDL
    destination->x = (int)source->x * 0x100;
    destination->y = (int)source->y * 0x100;
    destination->z = (int)source->z * 0x100;
#else
    destination->x = (int)source->x << 8;
    destination->y = (int)source->y << 8;
    destination->z = (int)source->z << 8;
#endif
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
    NormalizeFixedVector(&left);
    NormalizeFixedVector(&right);
    return (short)(((short)dot_product(&left, &right) * 100) / 0x100);
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

    normalized = *vector;
    NormalizeFixedVector(&normalized);
    return MultiplyFixed(
        Vector_magnitude(vector), dot_product(direction, &normalized));
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

/* Function start: 0x40B02F */
void rotate_about_i(short angle, FixedVector *j, FixedVector *k)
{
    int old;
    int cosine;
    int sine;

    cosine = CosFixed(angle);
    sine = SinFixed(angle);
    old = j->x;
    j->x = MultiplyFixed(old, cosine) - MultiplyFixed(k->x, sine);
    k->x = MultiplyFixed(old, sine) + MultiplyFixed(k->x, cosine);
    old = j->y;
    j->y = MultiplyFixed(old, cosine) - MultiplyFixed(k->y, sine);
    k->y = MultiplyFixed(old, sine) + MultiplyFixed(k->y, cosine);
    old = j->z;
    j->z = MultiplyFixed(old, cosine) - MultiplyFixed(k->z, sine);
    k->z = MultiplyFixed(old, sine) + MultiplyFixed(k->z, cosine);
}

/* Function start: 0x40B17F */
void rotate_about_j(short angle, FixedVector *i, FixedVector *k)
{
    int cosine;
    int sine;
    int old;

    cosine = CosFixed(angle);
    sine = SinFixed(angle);
    old = i->x;
    i->x = MultiplyFixed(k->x, sine) + MultiplyFixed(old, cosine);
    k->x = MultiplyFixed(k->x, cosine) - MultiplyFixed(old, sine);
    old = i->y;
    i->y = MultiplyFixed(k->y, sine) + MultiplyFixed(old, cosine);
    k->y = MultiplyFixed(k->y, cosine) - MultiplyFixed(old, sine);
    old = i->z;
    i->z = MultiplyFixed(k->z, sine) + MultiplyFixed(old, cosine);
    k->z = MultiplyFixed(k->z, cosine) - MultiplyFixed(old, sine);
}

/* Function start: 0x40B2CF */
void rotate_about_k(short angle, FixedVector *i, FixedVector *j)
{
    int old;
    int cosine;
    int sine;

    cosine = CosFixed(angle);
    sine = SinFixed(angle);
    old = i->x;
    i->x = MultiplyFixed(old, cosine) - MultiplyFixed(j->x, sine);
    j->x = MultiplyFixed(old, sine) + MultiplyFixed(j->x, cosine);
    old = i->y;
    i->y = MultiplyFixed(old, cosine) - MultiplyFixed(j->y, sine);
    j->y = MultiplyFixed(old, sine) + MultiplyFixed(j->y, cosine);
    old = i->z;
    i->z = MultiplyFixed(old, cosine) - MultiplyFixed(j->z, sine);
    j->z = MultiplyFixed(old, sine) + MultiplyFixed(j->z, cosine);
}

/* Function start: 0x40B41F */
void init_ijk(short obj)
{
    g_aShipForwardVector_00494208[obj].z = 0x100;
    g_aShipUpVector_00493ec0[obj].y = 0x100;
    g_aShipRightVector_00493b78[obj].x = 0x100;
    g_aShipForwardVector_00494208[obj].y = 0;
    g_aShipUpVector_00493ec0[obj].z = 0;
    g_aShipForwardVector_00494208[obj].x = 0;
    g_aShipUpVector_00493ec0[obj].x = 0;
    g_aShipRightVector_00493b78[obj].z = 0;
    g_aShipRightVector_00493b78[obj].y = 0;
    g_anObjectRollRotation_00495058[obj] = 0;
    g_anObjectYawRotation_00494fc8[obj] = 0;
    g_anObjectPitchRotation_00494f38[obj] = 0;
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

/* Function start: 0x40B747 */
void alter_pitch(short angle, short obj)
{
    rotate_about_i(angle, &g_aShipUpVector_00493ec0[obj],
                   &g_aShipForwardVector_00494208[obj]);
    fix_objects_ijk(obj);
}

/* Function start: 0x40B706 */
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
    long magnitude;

    ComputeVectorDelta((FixedVector *)from, (FixedVector *)to, &delta);
    magnitude = Vector_magnitude(&delta);
    return FixedToShortSaturating((int)magnitude);
}

/* Function start: 0x40B805 */
short distance_from_point(short obj, const FixedVector *point)
{
    long magnitude;

    ComputeVectorDelta(&g_aShipPosition_00494550[obj],
                       (FixedVector *)point, &g_vToTarget_00493188);
    magnitude = Vector_magnitude(&g_vToTarget_00493188);
    return FixedToShortSaturating((int)magnitude) -
           g_asObjectCollisionRadius_004950e8[obj];
}

/* Function start: 0x40B85C */
short distance_from_object(short obj, short other)
{
    return distance_from_point(obj, &g_aShipPosition_00494550[other]) -
           g_asObjectCollisionRadius_004950e8[other];
}

/* Function start: 0x40B898 */
void get_facing_range_from_point(short obj, const FixedVector *point)
{
    g_nTargetRange_0049319c =
        distance_from_point(obj, point) -
        g_asObjectCollisionRadius_004950e8[obj];
    g_vNormalizedToTarget_005d3bd0 = g_vToTarget_00493188;
    NormalizeFixedVector(&g_vNormalizedToTarget_005d3bd0);
    g_nFacingToTarget_00493194 =
        (short)(((short)dot_product(
            &g_vNormalizedToTarget_005d3bd0,
            &g_aShipForwardVector_00494208[obj]) * 100) >> 8);
}

/* Function start: 0x40B92F */
void get_facing_range_from_object(short obj, short other)
{
    get_facing_range_from_point(obj, &g_aShipPosition_00494550[other]);
    g_nTargetRange_0049319c -= g_asObjectCollisionRadius_004950e8[other];
    negate_vector(&g_vNormalizedToTarget_005d3bd0);
    g_nTargetFacing_00493198 =
        (short)(((short)dot_product(
            &g_vNormalizedToTarget_005d3bd0,
            &g_aShipForwardVector_00494208[other]) * 100) >> 8);
}

/* Function start: WC2_UNMAPPED */
void ship_vs_point(short obj, const FixedVector *point)
{
    get_facing_range_from_point(obj, point);
}

/* Function start: WC2_UNMAPPED */
void ship_vs_ship(short obj, short other)
{
    get_facing_range_from_object(obj, other);
}

/* Function start: 0x40B9B5 */
short facing_to_object(short obj, FixedVector *point)
{
    FixedVector direction;

    ComputeVectorDelta(&g_aShipPosition_00494550[obj],
                       point, &direction);
    NormalizeFixedVector(&direction);
    g_nFacingToTarget_00493194 =
        (short)(((short)dot_product(
            &direction, &g_aShipForwardVector_00494208[obj]) * 100) >> 8);
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
int set_ship_rotation_goals(short obj, short turnRate,
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
        if (spherical.yaw <= 0)
            spherical.yaw = (short)(spherical.yaw + turnRate);
        else
            spherical.yaw = (short)(spherical.yaw - turnRate);
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
            if (spherical.pitch <= 0)
                spherical.pitch = (short)(spherical.pitch - 180);
            else
                spherical.pitch = (short)(spherical.pitch + 180);
        }
        if (spherical.pitch <= 0)
            spherical.pitch = (short)(spherical.pitch + turnRate);
        else
            spherical.pitch = (short)(spherical.pitch - turnRate);
    }
    *yawGoal = WrapDegrees((short)-spherical.yaw);
    *pitchGoal = WrapDegrees((short)-spherical.pitch);
    return 0;
}

/* Function start: 0x40BC6B */
void point_ship(short obj, short turnRate, const FixedVector *direction)
{
    set_ship_rotation_goals(obj, turnRate, direction,
                            g_acShipPointingMode_0059d790[obj],
                            &g_anYawGoal_004954c0[obj],
                            &g_anPitchGoal_004954a8[obj]);
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

/* Function start: 0x40BDC2 */
void point_ship_behind_object(short obj, short other)
{
    FixedVector point = g_aShipPosition_00494550[other];

    position_relative(&point, g_aShipForwardVector_00494208[other],
                      (short)(-500 -
                          g_asObjectCollisionRadius_004950e8[other]));
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

/* Function start: 0x40BD58 */
void point_perpendicular_to_point(short obj, const FixedVector *point)
{
    point_ship_at_point(obj, point);
    if (g_anYawGoal_004954c0[obj] < 0)
        g_anYawGoal_004954c0[obj] += 90;
    else
        g_anYawGoal_004954c0[obj] -= 90;
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

/* Function start: WC2_UNMAPPED */
void MoveObjectAlongDirection(short obj, const FixedVector *direction,
                              short distance)
{
    FixedVector offset;

    offset = *direction;
    SetVectorFixedPoint((unsigned int *)&offset, distance);
    AddFixedVectors(&g_aShipPosition_00494550[obj], &offset,
                    &g_aShipPosition_00494550[obj]);
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
#ifdef WC1_SDL
    NormalizeAndScaleVector((FixedVector *)p, (int)v * 0x100);
#else
    NormalizeAndScaleVector((FixedVector *)p, (int)v << 8);
#endif
}

/* Function start: 0x40BF55 */
unsigned int IsPointWithinRange(FixedVector *from, FixedVector *to, short range)
{
    FixedVector delta;

    ComputeVectorDelta(from, to, &delta);
    return IsVectorWithinRange(&delta, range);
}

/* Function start: 0x40BF8C */
short check_for_collision(short obj)
{
#if 0
    FixedVector *objectPosition;
    FixedVector *position;
    int objectIndex;
    short other;
    short range;

    objectIndex = (int)obj;
    objectPosition = &g_aShipPosition_00494550[objectIndex];
    other = 0;
    position = g_aShipPosition_00494550;
    do {
        if (other != obj &&
            g_aeObjectClass_00495328[(int)other] >=
                OBJECT_CLASS_PROJECTILE) {
            ComputeVectorDelta(objectPosition, position,
                               &g_vCollisionDelta_00493178);
            range = (short)(
                g_asObjectCollisionRadius_004950e8[(int)other] +
                g_asObjectCollisionRadius_004950e8[objectIndex]);
            if (g_aeObjectClass_00495328[(int)other] ==
                    OBJECT_CLASS_SHIP &&
                g_aeObjectClass_00495328[objectIndex] == OBJECT_CLASS_SHIP)
                range >>= 1;
            if (IsVectorWithinRange(&g_vCollisionDelta_00493178,
                                    range) != 0)
                return other;
        }
        other++;
        position++;
    } while (other <= WC1_SPACE_LAST_MOVING_OBJECT);
    return -1;
#else
    FixedVector *objectPosition;
    FixedVector *position;
    short other;
    short range;

    objectPosition = &g_aShipPosition_00494550[obj];
    position = g_aShipPosition_00494550;
    for (other = 0; other <= WC2_SPACE_LAST_MOVING_OBJECT;
         other++, position++) {
        if (other != obj &&
            g_aeObjectClass_00495328[other] >= OBJECT_CLASS_PROJECTILE) {
            ComputeVectorDelta(objectPosition, position,
                               &g_vCollisionDelta_00493178);
            range = (short)(g_asObjectCollisionRadius_004950e8[other] +
                            g_asObjectCollisionRadius_004950e8[obj]);
            if (g_aeObjectClass_00495328[other] == OBJECT_CLASS_SHIP &&
                g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP)
                range >>= 1;
            if (IsVectorWithinRange(&g_vCollisionDelta_00493178,
                                    range) != 0)
                return other;
        }
    }
    return -1;
#endif
}

/* Function start: 0x40C08C */
void position_child(short parent, short hardpoint, FixedVector *position)
{
    const ShortVector *offset = &g_aChildOffsets_004682f0[hardpoint];

    position->x = g_aShipForwardVector_00494208[parent].x * offset->z +
                  g_aShipUpVector_00493ec0[parent].x * offset->y +
                  g_aShipRightVector_00493b78[parent].x * offset->x +
                  g_aShipPosition_00494550[parent].x;
    position->y = g_aShipForwardVector_00494208[parent].y * offset->z +
                  g_aShipUpVector_00493ec0[parent].y * offset->y +
                  g_aShipRightVector_00493b78[parent].y * offset->x +
                  g_aShipPosition_00494550[parent].y;
    position->z = g_aShipForwardVector_00494208[parent].z * offset->z +
                  g_aShipUpVector_00493ec0[parent].z * offset->y +
                  g_aShipRightVector_00493b78[parent].z * offset->x +
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
#if 0
    short i = 10;

    do {
        if (g_aeObjectClass_00495328[i] == OBJECT_CLASS_NULL) {
            g_asObjectScreenX_00493598[i] = (short)0x8001;
            return i;
        }
        i = i + 1;
    } while (i <= 0x3c);
    return -1;
#else
    short object;

    for (object = 10; object <= WC2_SPACE_LAST_MOVING_OBJECT; object++) {
        if (g_aeObjectClass_00495328[object] == OBJECT_CLASS_NULL) {
            g_asObjectScreenX_00493598[object] = (short)0x8001;
            return object;
        }
    }
    return -1;
#endif
}

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
        if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_CAPITAL_SHIP)
            FreePacketAndClear(&g_apObjectShape_00493868[obj], 0);
        g_acShipRating_0059cd80[obj] = -1;
        g_acShipPendingMessage_00495d98[obj] = -1;
        g_asShipSide_004955d0[obj] = SIDE_NEUTRAL;
        g_asShipManeuver_00495f48[obj] = MANEUVER_NONE;
        clear_alert(obj);
        g_asLoadedShipViewFrame_00495d18[obj] = -1;
    }
    g_aeObjectClass_00495328[obj] = OBJECT_CLASS_NULL;
    g_apObjectShape_00493868[obj] = 0;
}

/* Function start: 0x40C46C */
void apply_force_to_objects_center(FixedVector *force, short obj)
{
    FixedVector acceleration;

    divide_vector(force,
                  (unsigned short)g_asObjectRadarRadius_0059c790[obj]
                      << 8,
                  &acceleration);
    AddFixedVectors(&g_aShipVelocity_00494898[obj], &acceleration,
                    &g_aShipVelocity_00494898[obj]);
}

/* Function start: 0x40C4D1 */
void apply_force_to_object(FixedVector *point, FixedVector *force,
                           short obj)
{
    FixedVector localPoint;
    FixedVector localForce;
    FixedVector acceleration;
    int rotationalMass;
    int mass;
    int value;

    transform_to_objects_frame(force, &localForce, obj);
    transform_to_objects_frame(point, &localPoint, obj);
    rotationalMass = DivideFixed(
        (unsigned short)g_asObjectAfterburnerVelocity_0059c9d0[obj]
            << 8,
        (int)g_asObjectCollisionRadius_004950e8[obj] << 8);

    value = DivideFixed(
        MultiplyFixed(localPoint.x, localForce.y) -
            MultiplyFixed(localPoint.y, localForce.x),
        rotationalMass);
    g_anObjectRollRotation_00495058[obj] += (short)(value >> 8);
    value = DivideFixed(
        MultiplyFixed(localPoint.x, localForce.z) -
            MultiplyFixed(localPoint.z, localForce.x),
        rotationalMass);
    g_anObjectYawRotation_00494fc8[obj] += (short)(value >> 8);
    value = DivideFixed(
        MultiplyFixed(localPoint.z, localForce.y) -
            MultiplyFixed(localPoint.y, localForce.z),
        rotationalMass);
    g_anObjectPitchRotation_00494f38[obj] += (short)(value >> 8);
    ClampTo30(&g_anObjectPitchRotation_00494f38[obj]);
    ClampTo30(&g_anObjectYawRotation_00494fc8[obj]);
    ClampTo30(&g_anObjectRollRotation_00495058[obj]);

    mass = (unsigned short)g_asObjectRadarRadius_0059c790[obj] << 8;
    acceleration.x = DivideFixed(
        MultiplyFixed(0x16a -
            PlanarMagnitude(localPoint.y, localPoint.z),
            localForce.x),
        MultiplyFixed(0x16a, mass));
    acceleration.y = DivideFixed(
        MultiplyFixed(0x16a -
            PlanarMagnitude(localPoint.x, localPoint.z),
            localForce.y),
        MultiplyFixed(0x16a, mass));
    acceleration.z = DivideFixed(
        MultiplyFixed(0x16a -
            PlanarMagnitude(localPoint.x, localPoint.y),
            localForce.z),
        MultiplyFixed(0x16a, mass));
    AddFixedVectors(&g_aShipVelocity_00494898[obj], &acceleration,
                    &g_aShipVelocity_00494898[obj]);
    if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP)
        check_for_lost_control(obj);
}

/* Function start: 0x40C796 */
void rotational_acceleration(FixedVector *point, FixedVector *force,
                             short obj)
{
    FixedVector localPoint;
    FixedVector localForce;
    int denominator;
    int value;

    transform_to_objects_frame(force, &localForce, obj);
    transform_to_objects_frame(point, &localPoint, obj);
    denominator = DivideFixed(
        (unsigned short)g_asObjectAfterburnerVelocity_0059c9d0[obj]
            << 8,
        MultiplyFixed(
            (int)g_asObjectCollisionRadius_004950e8[obj] << 8,
            0x123c));

    value = DivideFixed(
        MultiplyFixed(localPoint.x, localForce.y) -
            MultiplyFixed(localPoint.y, localForce.x),
        denominator);
    g_anObjectRollRotation_00495058[obj] += (short)(value >> 8);
    value = DivideFixed(
        MultiplyFixed(localPoint.x, localForce.z) -
            MultiplyFixed(localPoint.z, localForce.x),
        denominator);
    g_anObjectYawRotation_00494fc8[obj] += (short)(value >> 8);
    value = DivideFixed(
        MultiplyFixed(localPoint.z, localForce.y) -
            MultiplyFixed(localPoint.y, localForce.z),
        denominator);
    g_anObjectPitchRotation_00494f38[obj] += (short)(value >> 8);
    ClampTo30(&g_anObjectPitchRotation_00494f38[obj]);
    ClampTo30(&g_anObjectYawRotation_00494fc8[obj]);
    ClampTo30(&g_anObjectRollRotation_00495058[obj]);
    if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP)
        check_for_lost_control(obj);
}

/* Function start: 0x40C959 */
void ClampVectorTo30(short *p)
{
#if 0
    short v = *p;

    if (v < 0) {
        *p = v + 1;
        return;
    }
    if (0 < v)
        *p = v - 1;
#else
    if (*p < 0) {
        *p = *p + 1;
    } else if (*p > 0) {
        *p = *p - 1;
    }
#endif
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

/* Function start: WC2_UNMAPPED */
unsigned short IsPointWithinEyeViewCone(const FixedVector *point)
{
    FixedVector direction;
    FixedVector viewPosition;
    int distance;
    long projection;
    unsigned short visible;

    ComputeVectorDelta(&g_aShipPosition_00494550[WC2_EYE_OBJECT],
                       (FixedVector *)point, &direction);
    distance = Vector_magnitude(&direction);
    if (g_asObjectCollisionRadius_004950e8[WC2_EYE_OBJECT] * 0x100 >
        distance)
        return 0;
    transform_to_objects_frame(&direction, &viewPosition,
                               WC2_EYE_OBJECT);
    if (g_asObjectCollisionRadius_004950e8[WC2_EYE_OBJECT] * 0x100 >
        viewPosition.z)
        return 0;
    projection = DivideFixed(viewPosition.z, distance);
    visible = 0;
    if (projection >= 0x94)
        visible = 1;
    return visible;
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
    for (obj = 0; obj <= WC2_SPACE_LAST_MOVING_OBJECT; obj++) {
        if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_NULL)
            continue;
        if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_FIXED_OBJECT)
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
                ComputeVectorDelta(&g_aShipPosition_00494550[WC2_EYE_OBJECT],
                                   &g_aShipPosition_00494550[obj],
                                   &direction);
            }
            distance = Vector_magnitude(&direction);
            if (distance <
                g_asObjectCollisionRadius_004950e8[WC2_EYE_OBJECT] * 0x100) {
                g_asObjectScreenX_00493598[obj] = (short)0x8001;
                continue;
            }
            if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_DUST &&
                distance > 0x57800) {
                g_asObjectScreenX_00493598[obj] = (short)0x8001;
                continue;
            }
            transform_to_objects_frame(&direction, &viewPosition,
                                       WC2_EYE_OBJECT);
            if (viewPosition.z <
                g_asObjectCollisionRadius_004950e8[WC2_EYE_OBJECT] * 0x100) {
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
                        g_asObjectCollisionRadius_004950e8[WC2_EYE_OBJECT] << 8,
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
    FixedVector projectedUp;
    short angle;

    negate_vector(direction);
    rectangular_to_spherical(direction, &spherical);
    init_ijk(WC2_SCRATCH_VIEW_OBJECT);
    alter_yaw((short)-spherical.yaw, WC2_SCRATCH_VIEW_OBJECT);
    alter_pitch((short)-spherical.pitch, WC2_SCRATCH_VIEW_OBJECT);
    projectedUp.x = dot_product(
        &g_aShipUpVector_00493ec0[WC2_EYE_OBJECT],
        &g_aShipRightVector_00493b78[WC2_SCRATCH_VIEW_OBJECT]);
    projectedUp.y = dot_product(
        &g_aShipUpVector_00493ec0[WC2_EYE_OBJECT],
        &g_aShipUpVector_00493ec0[WC2_SCRATCH_VIEW_OBJECT]);
    projectedUp.z = 0;
    NormalizeFixedVector(&projectedUp);
    angle = (short)ArcCos(projectedUp.y);
    if (projectedUp.x >= 0)
        angle = 360 - angle;
    g_asObjectScreenAngle_004936b8[obj] = angle;
    g_asObjectScreenScale_00493a58[obj] = 0xff;
}

/* Function start: 0x40CFF8 */
void get_right_shape(short obj, FixedVector *direction)
{
    FixedVector right = { 0x100, 0, 0 };
    FixedVector up = { 0, 0x100, 0 };
    FixedVector forward = { 0, 0, 0x100 };
    FixedVector objectForward;
    FixedVector eyeUp;
    FixedVector projectedUp;
    SphericalVector spherical;
    enum ObjectClass objectClass;
    enum ObjectType type;
    short pitchBand;
    short yawSector;
    short directionIndex;
    short frame;
    short remainder;
    short angle;
    short slot;

    negate_vector(direction);
    rectangular_to_spherical(direction, &spherical);
    rotate_about_j((short)-spherical.yaw, &right, &forward);
    rotate_about_i((short)-spherical.pitch, &up, &forward);
    NormalizeFixedVector(&up);
    NormalizeFixedVector(&forward);
    transform_to_objects_frame(&forward, &objectForward, obj);
    transform_to_objects_frame(&g_aShipUpVector_00493ec0[WC2_EYE_OBJECT],
                               &eyeUp, obj);
    rectangular_to_spherical(&objectForward, &spherical);
    pitchBand = (short)(spherical.pitch / 30 + 3);
    remainder = (short)(spherical.pitch % 30);
    if (remainder >= 16) {
        pitchBand++;
        if (pitchBand > 5)
            pitchBand = 6;
    } else if (remainder < -15) {
        pitchBand--;
        if (pitchBand < 1)
            pitchBand = 0;
    }
    yawSector = (short)((12 - spherical.yaw / 30) % 12);
    remainder = (short)(spherical.yaw % 30);
    if (remainder >= 16)
        yawSector = (short)((yawSector + 11) % 12);
    else if (remainder < -15)
        yawSector = (short)((yawSector + 1) % 12);
    if (yawSector < 0)
        yawSector += 12;
    if (pitchBand == 0)
        directionIndex = 0;
    else if (pitchBand == 6)
        directionIndex = 61;
    else
        directionIndex = (short)(pitchBand * 12 + yawSector - 11);

    projectedUp.x = dot_product(
        &eyeUp, &g_aDirectionViewRightVector_005a6c20[directionIndex]);
    projectedUp.y = dot_product(
        &eyeUp, &g_aDirectionViewUpVector_005a6f10[directionIndex]);
    projectedUp.z = 0;
    NormalizeFixedVector(&projectedUp);
    angle = (short)ArcCos(projectedUp.y);
    if (projectedUp.x >= 0)
        angle = (short)(360 - angle);

    objectClass = g_aeObjectClass_00495328[obj];
    type = g_acObjectType_00493980[obj];
    if (objectClass == OBJECT_CLASS_MISSILE ||
        type == OBJECT_TYPE_TURRET) {
        directionIndex += WC1_DIRECTION_VIEW_COUNT;
    } else if (type == OBJECT_TYPE_KILRATHI_BASE) {
        directionIndex += WC1_DIRECTION_VIEW_COUNT * 2;
    }
    frame = g_acDirectionShapeFrame_0049d558[directionIndex];
    if (frame == 0)
        angle += 90;
    if (frame == 36 &&
        objectClass != OBJECT_CLASS_MISSILE)
        angle -= 90;
    g_asObjectFlip_004939c8[obj] =
        (short)(g_acDirectionShapeFlip_0049d618[directionIndex] << 4);
    angle %= 360;
    if (angle < 0)
        angle += 360;
    g_asObjectScreenAngle_004936b8[obj] = angle;

    if (objectClass == OBJECT_CLASS_CAPITAL_SHIP) {
        if (g_asLoadedShipViewFrame_00495d18[obj] != frame) {
            for (slot = 1; slot < 3; slot++) {
                if (g_aObjectResourceSlots_00493398[slot].resourceType ==
                    (signed char)type) {
                    break;
                }
            }
            g_asObjectViewFrame_00493508[obj] = 0;
            g_asLoadedShipViewFrame_00495d18[obj] = frame;
            if (g_stViewBuffer_005d2b00.pixels != 0 &&
                IdentityWord(
                    (unsigned short)g_apObjectShape_00493868[obj]) == 0) {
                free_view_buffer();
            }
            if (g_aapPacketReferences_00465c88[slot][frame] != 0) {
                g_apObjectShape_00493868[obj] =
                    g_aapPacketReferences_00465c88[slot][frame];
            } else {
                if (g_stViewBuffer_005d2b00.pixels != 0)
                    free_view_buffer();
                g_cCapitalShipLogicalFile_005a7da0 =
                    (signed char)(type + 22);
                g_apObjectShape_00493868[obj] =
                    FetchDiskPacketRetrying(
                        (short)g_cCapitalShipLogicalFile_005a7da0,
                        g_asLoadedShipViewFrame_00495d18[obj], 0);
            }
            initialize_view_buffer();
        }
    } else {
        g_asObjectViewFrame_00493508[obj] = frame;
    }
}

/* Function start: 0x4593D6 */
short InitializeModalTextPanel(ModalTextPanel *panel, short fontIndex,
                               unsigned int topLeft,
                               unsigned int bottomRight,
                               short clearColour,
                               unsigned char backgroundColour,
                               short borderColour)
{
    memcpy(&panel->left, &topLeft, sizeof(topLeft));
    memcpy(&panel->right, &bottomRight, sizeof(bottomRight));
    panel->previousContext = g_pCurrentTextContext_005c8d1c;
    g_pCurrentTextContext_005c8d1c = &panel->context;
    panel->context = g_stDefaultTextContext_005d2d20;
    if (fontIndex == -1)
        fontIndex = 1;
    InitializeTextContextFromFont(&panel->context, fontIndex,
                                  (unsigned char)clearColour,
                                  backgroundColour);
    panel->viewport = g_stModalSourceViewport_005d2c50;
    memcpy(&panel->savedBackground.left, &panel->left,
           sizeof(topLeft) + sizeof(bottomRight));
    memcpy(&panel->viewport.left, &panel->left,
           sizeof(topLeft) + sizeof(bottomRight));
    if (AllocateViewport(&panel->savedBackground, clearColour, 0) == 0)
        return 0;
    CopyViewportContents(&panel->viewport, &panel->savedBackground);
    panel->context.text = g_szTextScratchBuffer_005d1c40;
    panel->context.viewport = &panel->viewport;
    ResetStringBuilder(&panel->context);
    EraseTextContextBackground(&panel->context);
    DrawViewportBorder(&panel->viewport, panel->left, panel->top,
                       panel->right, panel->bottom, borderColour);
    return 1;
}

/* Function start: 0x459527 */
void DrawModalTextPanel(ModalTextPanel *panel, short x, short y,
                        unsigned char alignment,
                        const char *format, ...)
{
    char text[84];

#ifdef WC1_SDL
    va_list arguments;

    va_start(arguments, format);
    vsprintf(text, format, arguments);
    va_end(arguments);
#else
    vsprintf(text, format, (char *)(&format + 1));
#endif
    SetTextCursor((unsigned short)(panel->left + x),
                  (unsigned short)(panel->top + y));
    panel->context.alignment = alignment;
    strcat(text, "%P");
    FormatTextBufferFromStart(text);
    MarkDibDirty();
    DIBslamReal();
}

/* Function start: 0x4595AA */
void RestoreModalTextPanel(ModalTextPanel *panel)
{
    CopyViewportContents(&panel->savedBackground, &panel->viewport);
    free_viewport(&panel->savedBackground);
    g_pCurrentTextContext_005c8d1c = panel->previousContext;
}

/* Function start: 0x4595EF */
short ShowModalTextPanel(short fontIndex, const char *format, ...)
{
    unsigned int topLeft;
    unsigned int bottomRight;
    short halfWidth;
    char text[52];

#ifdef WC1_SDL
    va_list arguments;

    va_start(arguments, format);
    vsprintf(text, format, arguments);
    va_end(arguments);
#else
    vsprintf(text, format, (char *)(&format + 1));
#endif
    topLeft = g_dwModalBoundsTopLeft_0049ca48;
    bottomRight = g_dwModalBoundsBottomRight_0049ca4c;
    if (g_pModalTextPanel_0049ca50 == 0) {
        g_pModalTextPanel_0049ca50 = AllocateTaggedMemory(sizeof(ModalTextPanel), 0);
    }
    if (g_pModalTextPanel_0049ca50 == 0)
        return 0;
    if (InitializeModalTextPanel(g_pModalTextPanel_0049ca50, fontIndex,
                                 topLeft, bottomRight,
                                 g_cSecondaryViewBufferColour_0049cb4c, g_cSecondaryViewBufferColour_0049cb4c,
                                 g_cSecondaryViewBufferColour_0049cb4c) == 0) {
        ReleasePacketHandle(g_pModalTextPanel_0049ca50);
        g_pModalTextPanel_0049ca50 = 0;
        return 0;
    }
    halfWidth = MeasureTextPixelWidthClamped(text);
    halfWidth = (short)(((int)halfWidth * 8 +
        (((int)halfWidth * 8 >> 31) & 15)) >> 4);
    RestoreModalTextPanel(g_pModalTextPanel_0049ca50);
    *(short *)&topLeft = (short)(159 - halfWidth);
    *(short *)&bottomRight = (short)(161 + halfWidth);
    if (InitializeModalTextPanel(g_pModalTextPanel_0049ca50, fontIndex,
                                 topLeft, bottomRight,
                                 g_bPrimaryViewBufferColour_0049cb50,
                                 g_abGamePaletteReservedColours_0049cb54[0],
                                 g_abGamePaletteReservedColours_0049cb54[8]) == 0) {
        ReleasePacketHandle(g_pModalTextPanel_0049ca50);
        g_pModalTextPanel_0049ca50 = 0;
        return 0;
    }
    DrawModalTextPanel(g_pModalTextPanel_0049ca50, 0, 6, 2, text);
    MarkDibDirty();
    DIBslamReal();
    return 1;
}

/* Function start: 0x45979B */
void ReleaseModalTextPanel(void)
{
    if (g_pModalTextPanel_0049ca50 != 0) {
        RestoreModalTextPanel(g_pModalTextPanel_0049ca50);
        ReleasePacketHandle(g_pModalTextPanel_0049ca50);
        g_pModalTextPanel_0049ca50 = 0;
        MarkDibDirty();
        DIBslamReal();
    }
}

/* Function start: WC2_UNMAPPED */
short AnySavedGames(void)
{
    SaveGameRecord gameRecord;
    short slot;
    short found;

    found = 0;
    slot = 0;
    do {
        if (LoadGame(slot, &gameRecord) != 0) {
            found = 1;
            if (gameRecord.campaign.campaignIndex > 0)
                DAT_005a7d9c = 1;
        }
        slot++;
    } while (slot < 8);
    return found;
}
