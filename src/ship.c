/*
 *  Space objects: creation, weapons, movement and damage.
 *
 *  Retail object 0x410020-0x41754F (link order; see docs/ORDER.md).
 */
#define SHIP_C
#include "game.h"

short g_bAiMissileFiringEnabled = 1; /* 0x00492d58 */
short g_bFriendlyFireWarningIssued = 0; /* 0x00492d5c */
short g_nLastAdaptiveDifficultyChangeFrame = 0; /* 0x00492d60 */
short g_nObjectType62Index = -1; /* 0x00492d64 */
short g_nObjectType63Index = -1; /* 0x00492d68 */
short g_nLastFriendlyFireObjectFrame = 0x0ee4; /* 0x00492d6c */
short g_aasPlayerDamageSystemTable[5][10] = {
    { 0, 8, 6, 5, 0, 3, 5, 5, 7, 6 },
    { 0, 8, 6, 5, 4, 3, 4, 0, 4, 4 },
    { 1, 2, 5, 2, 7, 3, 4, 7, 5, 1 },
    { 1, 4, 1, 5, 2, 3, 4, 7, 2, 1 },
    { 4, 4, 4, 4, 0, 8, 6, 5, 4, 0 }
}; /* 0x00492d70 */
static short g_aaeExplosionDebris[4][7] = {
    {
        OBJECT_DATA_PIPE, OBJECT_DATA_O_RING,
        OBJECT_DATA_GIRDER_CHUNK, OBJECT_DATA_SHIP_TUBING,
        OBJECT_DATA_METAL_SHEET, OBJECT_DATA_SHIP_WING,
        OBJECT_DATA_BURNING_DEBRIS
    },
    {
        OBJECT_DATA_O_RING, OBJECT_DATA_O_RING,
        OBJECT_DATA_GIRDER_CHUNK, OBJECT_DATA_SHIP_TUBING,
        OBJECT_DATA_SHIP_WING, OBJECT_DATA_GIRDER_CHUNK,
        OBJECT_DATA_BURNING_DEBRIS
    },
    {
        OBJECT_DATA_PIPE, OBJECT_DATA_O_RING,
        OBJECT_DATA_METAL_SHEET, OBJECT_DATA_SHIP_TUBING,
        OBJECT_DATA_METAL_SHEET, OBJECT_DATA_SHIP_WING,
        OBJECT_DATA_SHIP_TUBING
    },
    {
        OBJECT_DATA_BURNING_DEBRIS, OBJECT_DATA_SHIP_TUBING,
        OBJECT_DATA_METAL_SHEET, OBJECT_DATA_SHIP_WING,
        OBJECT_DATA_PIPE, OBJECT_DATA_O_RING,
        OBJECT_DATA_BURNING_DEBRIS
    }
}; /* 0x00492dd8 */
short g_aeShipHitDebrisTypes[4] = {
    OBJECT_DATA_GIRDER_CHUNK,
    OBJECT_DATA_SHIP_TUBING,
    OBJECT_DATA_O_RING
}; /* 0x00492e10 */
short g_bExplosionTraversalIdle = 1; /* 0x00492e18 */
#ifdef SDL_PORT
/* SO2.exe 0x0049EA60 / explode 0x00455758: nested blasts share one visit set. */
static short g_nSdlExplosionDepth;
#endif
signed char g_acGunRefireDelay[4] = {5, 9, 3, 4}; /* 0x00492e1c */

/* Function start: 0x410020 */
short CountShipProjectiles(short ship)
{
    short count;
    short obj;

    count = 0;
    for (obj = 0; obj < SPACE_OBJECT_COUNT; obj++) {
        if (g_aeObjectClass[obj] == OBJECT_CLASS_PROJECTILE &&
            g_acObjectOwner[obj] == ship)
            count++;
    }
    return count;
}

/* Function start: 0x41008E */
short ShipHasCloakingDevice(short ship)
{
    short weapon;
    unsigned char *weapons;

    weapons = g_aShipWeapons[ship];
    weapon = 0;
    for (; (short)(signed char)weapons[0] > weapon; weapon++) {
        if (((ShipWeaponSlot *)(weapons + 1))[weapon].type == 0x3c)
            return 1;
    }
    return 0;
}

/* Function start: 0x410102 */
short CanShipWeaponDamageTarget(short ship, short target)
{
    if (g_aeObjectClass[ship] == OBJECT_CLASS_SHIP) {
        if (g_aasShipShield[target][0] >= 1000 ||
            g_aasShipShield[target][1] >= 1000)
            return 0;
    }
    return 1;
}

/* Function start: 0x410161 */
short IsCapitalShipObject(short obj)
{
    if (g_aeObjectClass[obj] >= OBJECT_CLASS_CAPITAL_SHIP)
        return 1;
    return 0;
}

/* Function start: 0x410192 */
short HasShipCockpitGunDisplay(short ship)
{
    short weapon;
    short count;

    count = (short)(signed char)g_aShipWeapons[ship][0];
    weapon = 0;
    for (; count > weapon; weapon++) {
        if (((ShipWeaponSlot *)&g_aShipWeapons[ship][1])[
                weapon].type == 0x0b)
            return 1;
    }
    return 0;
}

/* Function start: 0x410215 */
short ShipHasTorpedo(short ship)
{
    unsigned char *weapons;
    short weapon;

    weapons = g_aShipWeapons[ship];
    weapon = 0;
    for (; (short)(signed char)weapons[0] > weapon; weapon++) {
        if (((ShipWeaponSlot *)(weapons + 1))[weapon].type == 0x13)
            return 1;
    }
    return 0;
}

/* Function start: 0x410289 */
short GetAdaptiveTurnRate(void)
{
    short turnRate;

#ifdef SDL_PORT
    /* Retail indexes past row zero into the adjacent difflevl.000 table.
     * Native globals need not be adjacent, so address the loaded table
     * directly for series 1-13. Invalid missions use the zero-rate fallback. */
    turnRate = 0;
    if (g_nCurrentLevel >= 0 &&
        g_nCurrentLevel < 4) {
        if (g_nCurSeries == 0) {
            turnRate = g_aasSeriesMissionTurnRate[0][
                g_nCurrentLevel];
        } else if (g_nCurSeries > 0 &&
                   g_nCurSeries <= 13) {
            turnRate = g_asDifficultyLevels[
                g_nCurSeries - 1][g_nCurrentLevel];
        }
    }
#else
    turnRate = g_asDifficultyLevels[
        g_nCurSeries - 1][g_nCurrentLevel];
#endif
    turnRate = (short)((int)turnRate * g_nAdaptiveDifficulty / 5);
    if (turnRate == 0)
        turnRate = 1;
    if (turnRate > 200)
        turnRate = 200;
    return turnRate;
}

/* Function start: 0x4102FD */
void IncreaseAdaptiveDifficulty(void)
{
#ifdef SDL_PORT
    if (g_bSdlBalancedDifficulty != 0)
        return;
#endif
    if (g_nSpaceFrame -
            g_nLastAdaptiveDifficultyChangeFrame < 20)
        return;
    g_nAdaptiveDifficulty++;
    if (g_nAdaptiveDifficulty > 10)
        g_nAdaptiveDifficulty = 10;
    g_nLastAdaptiveDifficultyChangeFrame =
        g_nSpaceFrame;
}

/* Function start: 0x410352 */
void DecreaseAdaptiveDifficulty(void)
{
#ifdef SDL_PORT
    if (g_bSdlBalancedDifficulty != 0)
        return;
#endif
    if (g_nSpaceFrame -
            g_nLastAdaptiveDifficultyChangeFrame < 20)
        return;
    g_nAdaptiveDifficulty--;
    if (g_nAdaptiveDifficulty < 0)
        g_nAdaptiveDifficulty = 0;
    g_nLastAdaptiveDifficultyChangeFrame =
        g_nSpaceFrame;
}

/* Function start: 0x4103A6 */
short initialize_object(short obj, short type, short owner,
                        short matchLoadedType)
{
    if (obj != -1) {
        set_objects_data(obj, type, owner, matchLoadedType);
        zero_vector(&g_aShipPosition[obj]);
        zero_vector(&g_aShipVelocity[obj]);
    }
    return obj;
}

/* Function start: 0x41040D */
short borrow_dust(void)
{
    short i;
    signed char found;

    found = 0;
    for (i = 10; i <= 66; i++) {
        if (g_aeObjectClass[i] == OBJECT_CLASS_DUST) {
            found++;
            break;
        }
    }
    if (!found) {
        for (i = 10; i <= 66; i++) {
            if (g_aeObjectClass[i] == OBJECT_CLASS_DEBRIS) {
                found++;
                break;
            }
        }
    }
    if (!found) {
        for (i = 10; i <= 66; i++) {
            if (g_aeObjectClass[i] == OBJECT_CLASS_PROJECTILE &&
                g_acObjectOwner[i] != 0 &&
                g_asObjectScreenX[i] == (short)0x8001) {
                found++;
                break;
            }
        }
    }
    if (!found) {
        for (i = 10; i <= 66; i++) {
            if ((g_aeObjectClass[i] == OBJECT_CLASS_ASTEROID ||
                 g_aeObjectClass[i] == OBJECT_CLASS_MINE) &&
                g_asObjectScreenX[i] == (short)0x8001) {
                found++;
                break;
            }
        }
    }
    if (!found) {
        i = -1;
    } else {
        g_asObjectScreenX[i] = (short)0x8001;
    }
    return i;

}

/* Function start: 0x4105BF */
short new_object(short type, short owner)
{
    short obj;

    obj = find_vacant_3d_object();
    if (obj == -1 && owner == 0 && type >= 7 && type < 11)
        obj = borrow_dust();
    return initialize_object(obj, type, owner, 0);
}

/* Function start: 0x41062D */
short initialize_ship(short type, short owner,
                      short matchLoadedType)
{
    short obj = get_ship_slot();

    if (obj != -1) {
        initialize_object(obj, type, owner, matchLoadedType);
        g_asShipSide[obj] = SIDE_NEUTRAL;
    }
    return obj;
}

/* Function start: 0x410680 */
short any_selected(unsigned char *loadout, short objectClass)
{
    short selected;
    short weapon;

    selected = 0;
    weapon = 0;
    for (; (short)(signed char)loadout[0] > weapon && selected == 0;
         weapon++) {
        if (g_aObjectTypeData[
                ((ShipWeaponSlot *)(loadout + 1))[weapon].weaponType]
                .objectClass == objectClass &&
            ((ShipWeaponSlot *)(loadout + 1))[weapon].disabled == 0)
            selected = 1;
    }
    return selected;
}

/* Function start: 0x410715 */
void remove_weapon(short obj, short weapon)
{
    short removedType;
    short objectClass;
    short dataType;
    struct ShipLoadout *slots;
    short index;

    slots = &((struct ShipLoadout *)g_aShipWeapons)[obj];
    dataType = ((ShipWeaponSlot *)(g_aShipWeapons[obj] + 1))[
        weapon].weaponType;
    removedType = ((ShipWeaponSlot *)(g_aShipWeapons[obj] + 1))[
        weapon].type;
    objectClass = g_aObjectTypeData[dataType].objectClass;
    if (slots->count - 1 > weapon) {
        for (index = weapon; slots->count - 1 > index; index++) {
            slots->slots[index].type = slots->slots[index + 1].type;
            slots->slots[index].mount = slots->slots[index + 1].mount;
            slots->slots[index].disabled = slots->slots[index + 1].disabled;
            slots->slots[index].weaponType =
                slots->slots[index + 1].weaponType;
            if (obj == 0) {
                g_116_aWeaponDisplayPositions[index] =
                    g_116_aWeaponDisplayPositions[index + 1];
            }
        }
        if (slots->count < 0x10)
            slots->slots[slots->count].disabled = 1;
    }
    slots->count--;
    if (obj == 0) {
        if (any_selected((unsigned char *)slots, objectClass) == 0) {
            if (objectClass == OBJECT_CLASS_PROJECTILE) {
                select_new_gun();
            } else {
                g_nSelectedReleaseWeaponIndex = -1;
                select_new_release_weapon(removedType);
            }
        }
        if (get_mode(0) == 1)
            InvalidateVduMode(0);
    }
}

/* Function start: 0x41090F */
void InitializeShipWeaponTypeIndices(short obj)
{
    short weaponCount;
    short weaponIndex;

    weaponCount = (short)(signed char)
        g_aObjectTypeData[
            g_acObjectType[obj]].weaponLoadout[0];
    for (weaponIndex = 0; weaponIndex < weaponCount; weaponIndex++) {
        g_aShipWeapons[obj][
            weaponIndex * sizeof(ShipWeaponSlot) + 1 +
            offsetof(ShipWeaponSlot, weaponType)] =
            g_aShipWeapons[obj][
                weaponIndex * sizeof(ShipWeaponSlot) + 1 +
                offsetof(ShipWeaponSlot, type)];
#ifdef SDL_PORT
        /* SO2.exe 0x45168F: serialized Mace type 64 uses table slot 49. */
        if (((ShipWeaponSlot *)(g_aShipWeapons[obj] + 1))[
                weaponIndex].type == OBJECT_TYPE_MACE)
            ((ShipWeaponSlot *)(g_aShipWeapons[obj] + 1))[
                weaponIndex].weaponType = OBJECT_DATA_MACE;
#endif
    }
}

/* Function start: 0x410999 */
void set_objects_data(short obj, short type, short owner,
                      short matchResourceType)
{
    short frameIndex;
    unsigned char *animation;
    ObjectTypeData *source;
    short weapon;
    ObjectTypeData *typeData;

    g_1_asObjectCreationFrameTable[obj] = g_nSpaceFrame;
    g_abProjectileCollisionBonus[obj] = 0;
    if (matchResourceType != 0) {
        for (weapon = 0; weapon < 5; weapon++) {
            if (g_aObjectTypeData[weapon].resourceType == type)
                break;
        }
        typeData = &g_aObjectTypeData[weapon];
        g_acObjectType[obj] = (signed char)weapon;
        g_apObjectExhaustShape[obj] = typeData->animation;
        g_apObjectShape[obj] = typeData->shapeSet;
    } else {
        if (type == OBJECT_DATA_SPACE_DUST) {
            g_asObjectType[obj] = OBJECT_DATA_SPACE_DUST;
            g_aeObjectClass[obj] = OBJECT_CLASS_DUST;
            return;
        }
        if (
#ifdef SDL_PORT
            /* Special resource IDs (data capsule 61 and cockpit objects
             * 62/63) are mapped to table slots below. They have no fallback
             * case here and must not index the table before that mapping. */
            type >= 0 && type < OBJECT_DATA_COUNT &&
#endif
            g_aObjectTypeData[type].shapeSet == 0) {
            switch (type) {
            case OBJECT_DATA_EXPLOSION_MEDIUM:
                type = OBJECT_DATA_EXPLOSION_SMALL;
                break;
            case OBJECT_DATA_EXPLOSION_LARGE:
                type = OBJECT_DATA_EXPLOSION_SMALL;
                break;
            case OBJECT_DATA_SHIP_WING:
                type = OBJECT_DATA_PIPE;
                break;
            case OBJECT_DATA_METAL_SHEET:
                type = OBJECT_DATA_GIRDER_CHUNK;
                break;
            case OBJECT_DATA_ASTEROID2:
                type = OBJECT_DATA_ASTEROID1;
                break;
            case OBJECT_DATA_ASTEROID4:
                type = OBJECT_DATA_ASTEROID3;
                break;
            case OBJECT_DATA_ASTEROID6:
                type = OBJECT_DATA_ASTEROID5;
                break;
            }
        }
        if (type == 0x3d)
            g_acObjectType[obj] = OBJECT_DATA_SPACE_DUST;
        else
            g_acObjectType[obj] = (signed char)type;
        if (type == 0x3e) {
            g_acObjectType[obj] =
                (signed char)g_nObjectType62Index;
        }
        if (type == 0x3f) {
            g_acObjectType[obj] =
                (signed char)g_nObjectType63Index;
        }
#ifdef SDL_PORT
        /* SO2.exe 0x45199B keeps resource types separate from table slots. */
        if (type == OBJECT_TYPE_MACE)
            g_acObjectType[obj] = OBJECT_DATA_MACE;
        if (type == OBJECT_TYPE_NUCLEAR_EXPLOSION)
            g_acObjectType[obj] = OBJECT_DATA_NUCLEAR_EXPLOSION;
#endif
        typeData =
            &g_aObjectTypeData[g_acObjectType[obj]];
        source = typeData;
        if (type == 0xe)
            source = &g_aObjectTypeData[10];
        if (type == 0xd)
            source = &g_aObjectTypeData[8];
        g_asObjectType[obj] = type;
        if (type == OBJECT_DATA_CHAFF_POD &&
            g_aObjectTypeData[20].shapeSet == 0) {
            g_apObjectShape[obj] =
                g_aObjectTypeData[9].shapeSet;
        }
        if (type == OBJECT_DATA_ROCK_CHUNK) {
            g_apObjectShape[obj] =
                g_aObjectTypeData[
                    OBJECT_DATA_ASTEROID1].shapeSet;
        } else {
            g_apObjectShape[obj] = source->shapeSet;
        }
    }
    g_asObjectType[obj] = typeData->field_18;
    g_aeObjectClass[obj] = typeData->objectClass;
    if (g_aeObjectClass[obj] == OBJECT_CLASS_MISSILE) {
        if (g_asShipSide[owner] == SIDE_KILRATHI && owner != 0 &&
            g_pGenericMissileShape != 0 &&
            g_pGenericMissileExhaustShape != 0 &&
            g_asObjectType[obj] != OBJECT_DATA_CHAFF_POD) {
            g_apObjectShape[obj] = g_pGenericMissileShape;
            g_apObjectExhaustShape[obj] =
                g_pGenericMissileExhaustShape;
        } else {
            g_apObjectExhaustShape[obj] = typeData->animation;
        }
    }
    init_ijk(obj);
    g_asObjectCollisionRadius[obj] = typeData->collisionRadius;
    g_asObjectScale[obj] = typeData->scale;
    if (type == 0xe) {
        g_asObjectScale[obj] =
            (unsigned short)(g_asObjectScale[obj] * 8);
    }
    if (type == 0xd) {
        g_asObjectScale[obj] =
            (unsigned short)(g_asObjectScale[obj] * 8);
    }
    g_asObjectDamage[obj] = 0;
    g_asObjectFlip[obj] = g_asObjectDamage[obj];
    g_asObjectScreenAngle[obj] = g_asObjectFlip[obj];
    g_acObjectOwner[obj] = (signed char)owner;
    g_acLastCollisionObject[obj] = -1;
    if (g_aeObjectClass[obj] >= OBJECT_CLASS_MISSILE) {
        g_asObjectViewFrame[obj] = 0;
        g_acShipTarget[obj] = -1;
        if (g_aeObjectClass[obj] >= OBJECT_CLASS_SHIP) {
            g_aasShipShield[obj][0] = typeData->shieldFore;
            g_aasShipMaximumShield[obj][0] =
                g_aasShipShield[obj][0];
            g_aasShipShield[obj][1] = typeData->shieldAft;
            g_aasShipMaximumShield[obj][1] =
                g_aasShipShield[obj][1];
            g_aasShipArmor[obj][0] = typeData->armorFront;
            g_aasShipArmor[obj][2] = typeData->armorLeft;
            g_aasShipArmor[obj][3] = typeData->armorRight;
            g_aasShipArmor[obj][1] = typeData->armorRear;
            g_anShipFuel[obj] = *(int *)&typeData->lifetime;
            g_acShipIonDriveDamage[obj] = 0;
            g_acShipDamage[obj] = 0;
            recalc_max_velocity(obj);
            g_acPilotHitPoints[obj] = 4;
            memcpy(g_aShipWeapons[obj], typeData->weaponLoadout,
                   sizeof(typeData->weaponLoadout));
            InitializeShipWeaponTypeIndices(obj);
            if (obj == 0) {
                g_nSelectedReleaseWeaponIndex = -1;
                g_nSelectedGunType = -1;
                weapon = (short)(signed char)g_aShipWeapons[obj][0];
                while (weapon-- > 0) {
                    if (((ShipWeaponSlot *)(
                             g_aShipWeapons[obj] + 1))[
                            weapon].disabled == 0) {
                        if (g_aObjectTypeData[
                                ((ShipWeaponSlot *)(
                                     g_aShipWeapons[obj] + 1))[
                                    weapon].weaponType].objectClass ==
                            OBJECT_CLASS_PROJECTILE) {
                            if (g_nSelectedGunType != -1 &&
                                ((ShipWeaponSlot *)(
                                     g_aShipWeapons[obj] + 1))[
                                    weapon].type !=
                                    g_nSelectedGunType) {
                                g_nSelectedGunType = 0x80;
                            } else {
                                g_nSelectedGunType =
                                    ((ShipWeaponSlot *)(
                                         g_aShipWeapons[obj] + 1))[
                                        weapon].type;
                            }
                        } else {
                            g_nSelectedReleaseWeaponIndex = weapon;
                        }
                    }
                }
            }
            if (HasShipCockpitGunDisplay(obj) != 0)
                g_asShipTurretFireEnabled[obj] = 1;
            else
                g_asShipTurretFireEnabled[obj] = 0;
            g_asShipWeaponEnergy[obj] = 100;
            g_acShipLastAttacker[obj] = -1;
        }
    } else {
        animation = typeData->animation;
        if (animation == 0) {
            frameIndex = 0;
            g_asObjectViewFrame[obj] =
                (short)(typeData->yawRate + frameIndex);
        } else {
            g_acObjectAnimationDelay[obj] = 1;
            g_asObjectAnimationIndex[obj] = 0;
            frameIndex = g_asObjectAnimationIndex[obj];
            animate_shape(obj);
        }
    }
}

#pragma function(abs)
/* Function start: 0x411172 */
unsigned int match_rotation_goal(short *rotation, short *goal,
                                 short totalError, short rate)
{
    if (totalError != 0) {
        if (*goal > 180)
            *goal = *goal - 360;
        if (*goal < -180)
            *goal = *goal + 360;
        rate = MaxShort(1, (short)(abs(*rotation - *goal) * rate /
                                   totalError));
        if (*goal != *rotation || abs(*rotation) > rate) {
            if (*goal > 0) {
                *rotation = (short)(*rotation +
                    MaxShort(
                        MinShort(
                            (short)(MinShort(*goal, rate) - *rotation),
                            rate),
                        (short)-rate));
            } else {
                *rotation = (short)(*rotation +
                    MinShort(
                        MaxShort(
                            (short)(MaxShort(*goal, (short)-rate) -
                                    *rotation),
                            (short)-rate),
                        rate));
            }
        }
    }
    if (*goal != 0) {
        if (*goal > 0)
            *goal = MaxShort((short)(*goal - *rotation), 0);
        else
            *goal = MinShort((short)(*goal - *rotation), 0);
    }
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x41133D */
void rotate_object_to_goal(short obj)
{
    short rate;
    short turnScale;
    short totalError;
    ObjectTypeData *typeData;

    typeData = &g_aObjectTypeData[g_acObjectType[obj]];
    if (g_aeSpecialManeuver[obj] ==
            SPECIAL_MANEUVER_BLOWING_UP) {
        if ((short)alert_flag(obj, 1) != 0) {
            set_special(obj, SPECIAL_MANEUVER_NONE);
        } else {
            /* The original passes a difficulty the callee ignores. */
            if (g_asObjectCounter[obj] == -1 &&
                skill_check(obj, 7) != 0)
                g_aeSpecialManeuver[obj] = SPECIAL_MANEUVER_NONE;
            return;
        }
    }
    totalError = (short)(abs(g_anObjectRollRotation[obj] -
                             g_anRollGoal[obj]) +
                         abs(g_anObjectYawRotation[obj] -
                             g_anYawGoal[obj]) +
                         abs(g_anObjectPitchRotation[obj] -
                             g_anPitchGoal[obj]));
    rate = typeData->yawRate;
    turnScale = MinShort(180, MaxShort(100, GetAdaptiveTurnRate()));
    if (g_asShipSide[obj] != g_asShipSide[0] &&
        g_aeObjectClass[obj] == OBJECT_CLASS_SHIP)
        rate = (short)(turnScale * rate / 100);
    match_rotation_goal(&g_anObjectPitchRotation[obj],
                        &g_anPitchGoal[obj], totalError,
                        typeData->yawRate);
    rate = typeData->pitchRate;
    if (g_asShipSide[obj] != g_asShipSide[0] &&
        g_aeObjectClass[obj] == OBJECT_CLASS_SHIP)
        rate = (short)(turnScale * rate / 100);
    match_rotation_goal(&g_anObjectYawRotation[obj],
                        &g_anYawGoal[obj], totalError,
                        typeData->pitchRate);
    rate = typeData->rollRate;
    if (g_asShipSide[obj] != g_asShipSide[0] &&
        g_aeObjectClass[obj] == OBJECT_CLASS_SHIP)
        rate = (short)(turnScale * rate / 100);
    match_rotation_goal(&g_anObjectRollRotation[obj],
                        &g_anRollGoal[obj], totalError,
                        typeData->rollRate);
}

/* Function start: 0x4115F8 */
/* A ship whose commanded rotation runs further ahead of its own turn rates
 * than its pilot can absorb tumbles out of control for a few frames. */
void check_for_lost_control(short obj)
{
    short turnScale;
    short type;
    short overshoot;
    short turnAbility;

    if (obj != 0 &&
        g_aeSpecialManeuver[obj] != SPECIAL_MANEUVER_UNKNOWN_9) {
        if (g_asShipSide[obj] != g_asShipSide[0] &&
            g_aeObjectClass[obj] == OBJECT_CLASS_SHIP)
            turnScale = MinShort(180, MaxShort(100, GetAdaptiveTurnRate()));
        else
            turnScale = 100;
        type = g_acObjectType[obj];
        turnAbility = (short)(g_aObjectTypeData[type].pitchRate *
                           turnScale / 100);
        turnAbility = (short)(g_aObjectTypeData[type].yawRate *
                           turnScale / 100 + turnAbility);
        turnAbility = (short)(g_aObjectTypeData[type].rollRate *
                           turnScale / 100 + turnAbility);
        /* These identities retain retail's roll/yaw/pitch call order in
         * CL 4.1, then fold away before instruction emission. */
        overshoot = (short)(~~(~~abs(g_anObjectRollRotation[obj]) +
                            abs(g_anObjectYawRotation[obj])) +
                            abs(g_anObjectPitchRotation[obj]));
        overshoot = (short)(overshoot / turnAbility);
        /* The original passes the overshoot the callee ignores. */
        if (skill_check(obj, overshoot) ==
            0) {
            set_special(obj, SPECIAL_MANEUVER_BLOWING_UP);
            g_asObjectCounter[obj] =
                (short)(RandomBelowOrEqual(6) + 5);
        }
    }
}

#pragma intrinsic(abs)
/* Function start: 0x4117AC */
void celerate(short ship, int delta)
{
    int maximumSpeed;

    maximumSpeed = (int)g_asShipMaximumVelocity[ship] << 8;
    g_anShipSpeed[ship] += delta;
    if (g_anShipSpeed[ship] > maximumSpeed)
        g_anShipSpeed[ship] = maximumSpeed;
    if (g_anShipSpeed[ship] < 0)
        g_anShipSpeed[ship] = 0;
}

#pragma function(abs)
/* Function start: 0x41181D */
unsigned int approach_speed(short ship, int targetSpeed)
{
    int delta;
    int acceleration;

    acceleration = GetShipAccelerationRate(ship);
    delta = targetSpeed - g_anShipSpeed[ship];

    if ((short)alert_flag(ship, 1))
        acceleration *= 2;
    if (abs(delta) > acceleration)
        delta = MultiplyFixed(SignFixed(delta), acceleration);
    celerate(ship, delta);
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x4118A9 */
void steady_object(short ship)
{
    g_anRollGoal[ship] = g_anPitchGoal[ship] =
        g_anYawGoal[ship] = 0;
}

/* Function start: 0x4118F2 */
short real_velocity(short obj)
{
    return FixedToShortSaturating(
        Vector_magnitude(&g_aShipVelocity[obj]));
}

/* Function start: 0x411922 */
unsigned int fix_velocity(short obj)
{
    ScaleFixedVector(&g_aShipForwardVector[obj],
                     g_anShipSpeed[obj],
                     &g_aShipVelocity[obj]);
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x41195F */
void SortViableTargetsByDistance(void)
{
    short outer;
    short j;
    short range;
    short temp;

    if (g_cViableTargetCount > 1) {
        for (outer = 0;
             outer < (short)g_cViableTargetCount - 1;
             outer++) {
            for (j = (short)(outer + 1);
                 j < (short)g_cViableTargetCount;
                 j++) {
                range = g_asViableTargetDistance[outer];
                if (g_asViableTargetDistance[j] < range) {
                    temp = g_acViableTarget[outer];
                    g_asViableTargetDistance[outer] =
                        g_asViableTargetDistance[j];
                    g_acViableTarget[outer] =
                        g_acViableTarget[j];
                    g_asViableTargetDistance[j] = range;
                    g_acViableTarget[j] = (signed char)temp;
                }
            }
        }
    }
}

/* Function start: 0x411A50 */
void send_appropriate_message(short attacker, short victim)
{
    if (g_acObjectOwner[attacker] != -1 &&
        g_aeObjectClass[
            (short)g_acObjectOwner[attacker]] >=
            OBJECT_CLASS_SHIP) {
        if (g_asShipSide[
                (short)g_acObjectOwner[attacker]] ==
                SIDE_IMPERIAL &&
            g_asShipSide[
                (short)g_acObjectOwner[attacker]] !=
                g_asShipSide[victim]) {
            if (g_aeObjectClass[victim] >=
                    OBJECT_CLASS_CAPITAL_SHIP &&
                g_aeSpecialManeuver[
                    (short)g_acObjectOwner[attacker]] !=
                    SPECIAL_MANEUVER_UNKNOWN_9)
                send_message(
                    (short)g_acObjectOwner[attacker], 6);
            if (((short)g_acObjectOwner[attacker] ==
                     g_nYourWingman ||
                 RandomBelowOrEqual(100) < 70) &&
                g_aeSpecialManeuver[
                    (short)g_acObjectOwner[attacker]] !=
                    SPECIAL_MANEUVER_UNKNOWN_9 &&
                g_aeObjectClass[victim] == OBJECT_CLASS_SHIP) {
                if ((short)g_acObjectOwner[attacker] ==
                    g_nYourWingman) {
                    if (g_asPlayerHitsCreditValues[victim] <=
                            g_asWingmanDamageCredit[victim] ||
                        g_aeObjectClass[attacker] ==
                            OBJECT_CLASS_MISSILE)
                        send_message(
                            (short)g_acObjectOwner[attacker], 5);
                } else {
                    send_message(
                        (short)g_acObjectOwner[attacker], 5);
                }
            }
        } else if (g_asShipSide[
                       (short)g_acObjectOwner[attacker]] !=
                       SIDE_IMPERIAL &&
                   g_asShipSide[
                       (short)g_acObjectOwner[attacker]] !=
                       g_asShipSide[victim] &&
                   g_aeSpecialManeuver[
                       (short)g_acObjectOwner[attacker]] !=
                       SPECIAL_MANEUVER_UNKNOWN_9) {
            send_message((short)g_acObjectOwner[attacker], 0x1c);
        }
    }
}

/* Function start: 0x411C72 */
short inflict_damage(short attacker, short victim, short damage,
                     const FixedVector *impactDirection)
{
    short *protection;
    int sideProjection;
    short ally;
    short destroyed;
    short quadrant;

    if (g_acObjectOwner[attacker] == 0)
        destroyed = 0;
    if (g_bApplyingCollisionDamage != 0 &&
        g_asShipSide[victim] == SIDE_KILRATHI &&
        g_acObjectOwner[attacker] == 0 &&
        RandomBelowOrEqual(9) == 0)
        IncreaseAdaptiveDifficulty();
    if (g_bApplyingCollisionDamage != 0 && victim == 0 &&
        g_acObjectOwner[attacker] != -1 &&
        g_asShipSide[
            (short)g_acObjectOwner[attacker]] ==
            SIDE_KILRATHI &&
        RandomBelowOrEqual(5) == 0)
        DecreaseAdaptiveDifficulty();
    if (g_bPlayerDamageEnabled == 0 && victim == 0)
        return 0;
    if ((g_aasShipShield[victim][0] >= 1000 ||
         g_aasShipShield[victim][1] >= 1000) &&
        attacker != -1 && g_asObjectType[attacker] != 0x0d &&
        g_asObjectType[attacker] != 0x0e &&
        g_asObjectType[attacker] != 0x13
#ifdef SDL_PORT
        /* SO2.exe 0x452C01: the Mace also penetrates phase shields. */
        && g_asObjectType[attacker] != OBJECT_TYPE_MACE
#endif
        )
        return 0;
    if (attacker != -1 &&
        g_aeObjectClass[attacker] == OBJECT_CLASS_PROJECTILE &&
        CanShipWeaponDamageTarget(
            (short)g_acObjectOwner[attacker], victim) == 0)
        return 0;
    if (damage == 0 ||
        g_aeSpecialManeuver[victim] ==
            SPECIAL_MANEUVER_UNKNOWN_9 ||
        g_aeObjectClass[victim] < OBJECT_CLASS_MISSILE)
        return 0;

    else if (g_aeObjectClass[victim] < OBJECT_CLASS_SHIP) {
        g_asObjectDamage[victim] =
            (short)(g_asObjectDamage[victim] + damage);
        if (g_aObjectTypeData[
                g_acObjectType[victim]].damageCapacity == -1)
            return 0;
        else if (g_aObjectTypeData[
                g_acObjectType[victim]].damageCapacity <=
            g_asObjectDamage[victim])
            return explode(attacker, victim);
    } else {
        if (attacker != -1 &&
            g_acObjectOwner[attacker] != -1 &&
            g_asShipSide[victim] == SIDE_KILRATHI &&
            g_asShipSide[
                (short)g_acObjectOwner[attacker]] ==
                SIDE_KILRATHI &&
            g_aeObjectClass[
                (short)g_acObjectOwner[attacker]] >=
                OBJECT_CLASS_CAPITAL_SHIP)
            return 0;
        if (victim == 0) {
            TriggerPlayerHitPaletteFlash();
#ifdef SDL_PORT
            SdlQueueJoystickDamageRumble(damage);
#endif
        }
        if (g_acObjectOwner[attacker] != -1 &&
            g_aeObjectClass[
                (short)g_acObjectOwner[attacker]] ==
                OBJECT_CLASS_SHIP &&
            g_asShipSide[
                (short)g_acObjectOwner[attacker]] ==
                g_asShipSide[victim])
            g_asShipFriendlyFireCooldown[
                (short)g_acObjectOwner[attacker]] = 40;
        if ((short)g_acObjectOwner[attacker] ==
            g_nYourWingman)
            g_asWingmanDamageCredit[victim]++;
        if (g_acObjectOwner[attacker] == 0)
            g_asPlayerHitsCreditValues[victim] =
                (short)(g_asPlayerHitsCreditValues[victim] + 2);
        if (attacker == g_nYourWingman && victim == 0)
            send_message(g_nYourWingman, 0x0c);
        if (attacker == 0 && victim == g_nYourWingman)
            send_message(g_nYourWingman, 0x0c);
        if (g_1_asObjectCreationFrameTable[attacker] !=
                g_nLastFriendlyFireObjectFrame &&
            attacker != -1 &&
            g_asShipSide[victim] == SIDE_IMPERIAL &&
            g_acObjectOwner[attacker] == 0 &&
            (g_aeObjectClass[attacker] != OBJECT_CLASS_MISSILE ||
             g_acShipTarget[attacker] == victim)) {
            if (kilrathi_near(0, 30000) != 0) {
                send_message(victim, 0x0c);
                g_nLastFriendlyFireObjectFrame =
                    g_1_asObjectCreationFrameTable[attacker];
            } else if (g_asShipSide[0] != SIDE_KILRATHI) {
                if (g_bFriendlyFireWarningIssued == 0) {
                    g_bFriendlyFireWarningIssued++;
                    send_message(victim, 0x0d);
                    g_nLastFriendlyFireObjectFrame =
                        g_1_asObjectCreationFrameTable[attacker];
                } else {
                    send_message(victim, 0x0e);
                    g_nLastFriendlyFireObjectFrame =
                        g_1_asObjectCreationFrameTable[attacker];
                    g_asShipSide[0] = SIDE_KILRATHI;
                    g_nYourWingman = -1;
                    for (ally = 1; ally < 10; ally++) {
                        if (g_asShipSide[ally] == SIDE_IMPERIAL) {
                            if (g_aeObjectClass[ally] ==
                                OBJECT_CLASS_SHIP) {
                                g_asShipMissionType[ally] =
                                    MISSION_TYPE_STRIKE;
                                g_asShipMissionParameter[ally] =
                                    g_asShipMissionIndex[0];
                                g_aeShipObjective[ally] =
                                    OBJECTIVE_DESTROY_SHIP;
                                g_acShipTarget[ally] = 0;
                            } else if (g_aeObjectClass[ally] >=
                                       OBJECT_CLASS_CAPITAL_SHIP) {
                                g_asShipMissionType[ally] =
                                    MISSION_TYPE_NONE;
                                g_asShipTactic[ally] =
                                    TACTIC_SELF_DEFENSE;
                            }
                        }
                    }
                }
            }
        }
        quadrant = (short)(dot_product(
            impactDirection, &g_aShipForwardVector[victim]) > 0);
        if (g_anShipCloakState[victim] != 1 &&
            g_asObjectType[attacker] != 0x13 &&
            g_aasShipShield[victim][0] < 1000 &&
            g_aasShipShield[victim][1] < 1000) {
            protection = &g_aasShipShield[victim][quadrant];
            damage = (short)(damage - *protection);
            if (damage > 0) {
                *protection = 0;
            } else {
                *protection = (short)-damage;
                if (attacker != -1 &&
                    g_aeObjectClass[attacker] ==
                        OBJECT_CLASS_PROJECTILE) {
                    if (victim == 0)
                        PlaySfxWaveFileByNumber(10, victim, 0);
                    else
                        PlaySfxWaveFileByNumber(10, victim, 0);
                }
                return 0;
            }
        }
        if (attacker != -1 &&
            g_aeObjectClass[attacker] ==
                OBJECT_CLASS_PROJECTILE) {
            if (victim == 0)
                PlaySfxWaveFileByNumber(9, victim, 0);
            else
                PlaySfxWaveFileByNumber(9, victim, 0);
        }
        sideProjection = dot_product(
            impactDirection, &g_aShipRightVector[victim]);
        if (sideProjection > 0xb5)
            quadrant = 3;
        else if (sideProjection < -0xb5)
            quadrant = 2;
        protection = &g_aasShipArmor[victim][quadrant];
        damage = (short)(damage - *protection);
        if (damage > 0) {
            *protection = 0;
            if (g_asObjectScreenX[victim] != (short)0x8001 &&
                g_aeObjectClass[victim] <
                    OBJECT_CLASS_CAPITAL_SHIP &&
                RandomBelowOrEqual(1) == 0)
                Create_ship_hit_debris(attacker, 1);
            if (RandomBelowOrEqual(99) == 0) {
                if (g_acObjectOwner[attacker] == 0) {
                    if (g_asShipSide[victim] == SIDE_KILRATHI)
                        send_message(victim, 0x1d);
                    destroyed = explode(attacker, victim);
                }
            } else {
                destroyed = internal_damage(
                    attacker,
                    (short)g_acObjectOwner[attacker],
                    victim, damage, quadrant);
            }
            if (destroyed == 1) {
                send_appropriate_message(attacker, victim);
                if (g_asShipSide[victim] == SIDE_KILRATHI &&
                    g_acObjectOwner[attacker] == 0)
                    IncreaseAdaptiveDifficulty();
                if (g_nYourWingman == victim)
                    DecreaseAdaptiveDifficulty();
            }
            return destroyed;
        } else {
            *protection = (short)-damage;
            return 0;
        }
    }
#ifdef SDL_PORT
    /* Keep the native fallback when a non-ship survives the hit. */
    return 0;
#endif
}

/* Function start: 0x41262D */
short pilot_hit(short obj)
{
    if (g_acPilotHitPoints[obj] > 0) {
        g_acPilotHitPoints[obj]--;
        if (g_acPilotHitPoints[obj] == 0) {
            if (obj == 0) {
                if (g_bPlayerDamageEnabled != 0 &&
                    g_bDeathSequenceActive == 0)
                    g_nArcadeState = 4;
                return -1;
            }
        } else if (skill_check(obj, 9) == 0 && obj != 0) {
            g_asObjectCounter[obj] =
                (short)(RandomBelowOrEqual(20) + 30);
            set_special(obj, SPECIAL_MANEUVER_BLOWING_UP);
        }
    }
    return g_acPilotHitPoints[obj];
}

/* Function start: 0x41270A */
short onboard_explosion(short obj)
{
    short debris;
    FixedVector offset;

    debris = find_vacant_3d_object();
    if (debris != -1) {
        ((void (__cdecl *)(short, enum ObjectType, short, short))
            set_objects_data)(debris, (enum ObjectType)0x26, obj, 0);
        g_asObjectScale[debris] <<= 1;
        g_aShipVelocity[debris] =
            g_aShipVelocity[obj];
        g_asObjectCounter[debris] = 6;
        MakeRandomVectorFixed(
            (short)(g_asObjectCollisionRadius[obj] >> 2),
            (short)(g_asObjectCollisionRadius[obj] >> 1),
            &offset);
        AddFixedVectors(&g_aShipPosition[obj], &offset,
                        &g_aShipPosition[debris]);
        RecordCannedSceneObjectEvent(debris, 0);
    }
    return debris != -1;
}

/* Function start: 0x41280B */
void call_enemy(short obj)
{
    short other;

    other = 0;
    for (; other < 10; other++) {
        if (g_aeObjectClass[other] >= OBJECT_CLASS_SHIP &&
            g_aeSpecialManeuver[other] !=
                SPECIAL_MANEUVER_UNKNOWN_9 &&
            g_asShipSide[obj] != g_asShipSide[other] &&
            RandomBelow(100) < 50)
            g_acShipTarget[other] = obj;
    }
}

/* CL 4.1 uses this declaration context when ordering the ship operands. */
short HasShipCockpitGunDisplay(short ship);
void rotate_object_to_goal(short obj);
short RandomBelow(short n);

/* Function start: 0x4128A7 */
short internal_damage(short attacker, short owner, short victim,
                      short damage, short quadrant)
{
    short system;
    short type;
    signed char weapon;
    short ship;
    short capacity;

    if (victim == 0)
        return your_internal_damage(attacker, owner, damage, quadrant);
    type = (short)g_acObjectType[victim];
    capacity = g_aObjectTypeData[type].damageCapacity;
    if (g_aeObjectClass[victim] >= OBJECT_CLASS_CAPITAL_SHIP) {
        if (g_asShipSide[victim] == SIDE_KILRATHI) {
            if (owner == 0)
                IncreaseAdaptiveDifficulty();
            if (g_asObjectType[attacker] == 0x0d ||
                g_asObjectType[attacker] == 0x0e) {
                g_asObjectDamage[victim] = (short)(
                    g_asObjectDamage[victim] + damage);
            } else if (g_asObjectType[attacker] == 0x13 &&
                       damage > 100) {
                g_asObjectDamage[victim] = (short)(
                    g_asCapitalHitDamageStepTable[victim] +
                    g_asObjectDamage[victim]);
            } else {
                g_asObjectDamage[victim] = (short)(
                    g_asObjectDamage[victim] +
                    MaxShort(1, (short)(damage >> 3)));
            }
            ship = attacker;
            if (owner != -1)
                ship = owner;
            if (ship != -1 && ship < 10 &&
                any_enemy(ship, 10000) == 0)
                call_enemy(ship);
        } else {
            if (g_asObjectType[attacker] == 0x0d ||
                g_asObjectType[attacker] == 0x0e) {
                g_asObjectDamage[victim] = (short)(
                    g_asObjectDamage[victim] + damage);
            } else {
                g_asObjectDamage[victim] = (short)(
                    g_asObjectDamage[victim] +
                    MaxShort(1, (short)(damage / 10)));
            }
            if (RandomBelowOrEqual(1000) < 35 && attacker != 0 &&
                owner != 0)
                send_message(victim, 4);
        }
        if (g_asObjectDamage[victim] >= capacity)
            return explode(attacker, victim);
        else
            onboard_explosion(victim);
        return 0;
    } else {
        if (g_asPilotLevel[victim] == 5) {
            damage = MinShort(RandomInRange(3, 4),
                              MaxShort(1, (short)(damage / 40)));
        } else {
            damage = MaxShort(1, (short)(damage / 6));
        }
        g_asObjectDamage[victim] =
            (short)(g_asObjectDamage[victim] + damage);

        while (damage > 0) {
            if (damage == 1 && g_acShipPortrait[victim] != -1)
                system = 4;
            else
                system = RandomBelowOrEqual(9);
            switch (system) {
            case 8:
                if (quadrant != 0)
                    break;
                if (g_acShipCommunicator[victim] != -1) {
                    damage--;
                    g_acShipCommunicator[victim] = -1;
                }
                break;
            case 0:
                /* The test has an empty body in the shipped game, so a pilot
                 * hit always costs a point of damage. */
                if (pilot_hit(victim) != 0) {
                }
                damage--;
                break;
            case 6:
                if (quadrant != 0)
                    break;
                if (g_acShipComputerDamage[victim] < 5) {
                    damage--;
                    g_acShipComputerDamage[victim]++;
                }
                break;
            case 5:
                if (quadrant != 0)
                    break;
                if ((signed char)g_aShipWeapons[victim][0] > 0) {
                    weapon = (signed char)RandomBelowOrEqual(
                        (short)((signed char)
                            g_aShipWeapons[victim][0] - 1));
                    remove_weapon(victim, (short)weapon);
                    damage--;
                }
                break;
            case 2:
                if (quadrant != 1)
                    break;
                return explode(attacker, victim);
            case 3:
                g_aasShipMaximumShield[victim][0] = 0;
                g_aasShipShield[victim][0] = 0;
                g_aasShipMaximumShield[victim][1] = 0;
                g_aasShipShield[victim][1] = 0;
                break;
            case 4:
                damage--;
                g_acShipDamage[victim]++;
                if (g_asShipSide[victim] == SIDE_KILRATHI &&
                    owner == 0)
                    IncreaseAdaptiveDifficulty();
                if ((short)g_acShipDamage[victim] > capacity)
                    return explode(attacker, victim);
                break;
            case 7:
                if (quadrant != 1)
                    break;
                damage--;
                drain_fuel(victim,
                           (short)(*(int *)&g_aObjectTypeData[
                                       type].lifetime / 4));
                if (RandomBelowOrEqual(1) != 0 ||
                    g_anShipFuel[victim] < 0)
                    return explode(attacker, victim);
                break;
            case 1:
                if (quadrant != 1)
                    break;
                damage--;
                damage_ion_drive(victim, 1, 3);
                break;
            }
        }
    }
    return 0;
}

/* Function start: 0x412F03 */
void revise_shields(short obj)
{
    g_aasShipMaximumShield[obj][0] =
        MaxShort(0, (short)(g_aasShipMaximumShield[obj][0] -
            (g_aObjectTypeData[
                g_acObjectType[obj]].shieldFore >> 2)));
    g_aasShipMaximumShield[obj][1] =
        MaxShort(0, (short)(g_aasShipMaximumShield[obj][0] -
            (g_aObjectTypeData[
                g_acObjectType[obj]].shieldAft >> 2)));
}

/* Function start: 0x412F90 */
void check_computer_damage(void)
{
    damage_your_component(3, 1, 3);
}

/* Function start: 0x412FA9 */
short IsOnlyShipGun(short ship, short weapon)
{
    short otherGuns;
    short candidate;

    otherGuns = 0;
    if (g_aObjectTypeData[
            (signed char)((ShipWeaponSlot *)
                (g_aShipWeapons[ship] + 1))[weapon].weaponType].
            objectClass == OBJECT_CLASS_PROJECTILE) {
        candidate = 0;
        for (; candidate <
             (signed char)g_aShipWeapons[ship][0];
             candidate++) {
            if (weapon != candidate &&
                g_aObjectTypeData[
                    (signed char)((ShipWeaponSlot *)
                        (g_aShipWeapons[ship] + 1))[
#ifdef SDL_PORT
                            /* Retail rechecks weapon instead of candidate. */
                            candidate
#else
                            weapon
#endif
                        ].
                            weaponType].objectClass ==
                    OBJECT_CLASS_PROJECTILE)
                otherGuns++;
        }
        return otherGuns == 0;
    } else
        return 0;
}

/* Function start: 0x413099 */
short your_internal_damage(short attacker, short owner, short damage,
                           short quadrant)
{
    signed char roll;
    signed char system;
    short group;
    short craftType;
    signed char weapon;

    if (g_aeObjectClass[attacker] ==
        OBJECT_CLASS_PROJECTILE) {
        if (quadrant != 1)
            group = 0;
        else
            group = 2;
        damage = MaxShort(1, (short)(damage >> 4));
    } else if (g_aeObjectClass[attacker] ==
                   OBJECT_CLASS_ASTEROID ||
               g_aeObjectClass[attacker] >=
                   OBJECT_CLASS_SHIP) {
        group = 4;
        damage = MaxShort(1, (short)(damage >> 7));
    } else {
        if (quadrant != 1)
            group = 1;
        else
            group = 3;
        damage = MaxShort(1, (short)(damage >> 5));
    }
    craftType = (short)g_acObjectType[0];
    roll = (signed char)RandomBelowOrEqual(10);
    g_asObjectDamage[0] =
        (short)(g_asObjectDamage[0] + damage);
    if (damage > 1) {
        DecreaseAdaptiveDifficulty();
        place_damage_on_cockpit(RandomBelowOrEqual(3));
    }

    while (damage > 0) {
        damage--;
        system = (signed char)g_aasPlayerDamageSystemTable[
            group][RandomBelowOrEqual(9)];
        switch (system) {
        case 8:
            if (quadrant != 0) {
                damage++;
                break;
            }
            if (roll < 7) {
                damage_your_component(4, 2, 3);
                if (g_acPlayerComponentDamage[4] >= 4)
                    g_acShipCommunicator[0] = -1;
            } else {
                damage_your_component(5, 4, 4);
            }
            break;
        case 0:
            if (roll < 4) {
                pilot_hit(0);
            } else if (roll < 7) {
                damage_your_component(7, 2, 4);
            } else {
                damage_your_component(6, 4, 4);
            }
            break;
        case 6:
            if (quadrant != 0) {
                damage++;
                break;
            }
            if (g_acShipComputerDamage[0] < 5) {
                g_acShipComputerDamage[0]++;
                check_computer_damage();
            }
            break;
        case 5:
            if (quadrant != 0) {
                damage++;
                break;
            }
            if ((signed char)g_aShipWeapons[0][0] > 1) {
                weapon = (signed char)RandomBelowOrEqual(
                    (short)((signed char)g_aShipWeapons[0][0] - 1));
                if (IsOnlyShipGun(0, (short)weapon) == 0) {
                    remove_weapon(0, (short)weapon);
                    ShowComponentHitHudMessage(
                        _("Weapon destroyed"),
                        g_cRedColour, 8);
                }
            }
            break;
        case 2:
            if (quadrant != 1) {
                damage++;
                break;
            }
            if (RandomBelowOrEqual(3) == 0)
                return explode(attacker, 0);
            if (damage_your_component(1, 1, 4) == 4)
                return explode(attacker, 0);
            break;
        case 3:
            if (roll < 9) {
                damage_your_component(2, 1, 4);
                revise_shields(0);
            } else {
                damage_your_component(8, 2, 4);
            }
            break;
        case 4:
            g_acShipDamage[0]++;
            if (g_acShipDamage[0] == 1) {
                /* A stray semicolon in the original: the retail test of
                 * pilot_hit() branches to the decrement either way. */
                if (pilot_hit(0) != 0)
                    ;
                damage--;
            } else if ((short)g_acShipDamage[0] >
                       g_aObjectTypeData[craftType].
                           damageCapacity) {
                return explode(attacker, 0);
            }
            break;
        case 7:
            drain_fuel(0,
                       (short)(*(int *)&g_aObjectTypeData[
                                   craftType].lifetime / 4));
            if (RandomBelowOrEqual(1) != 0 ||
                g_anShipFuel[0] < 0)
                return explode(attacker, 0);
            ShowComponentHitHudMessage(
                _("Fuel tanks hit"),
                g_cRedColour, 8);
            break;
        case 1:
            if (quadrant != 1) {
                damage++;
                break;
            }
            damage_your_component(0, 1, 3);
            damage_ion_drive(0, 1, 3);
            break;
        }
    }
    return 0;
}

/* Function start: 0x413578 */
short ReportComponentRepaired(short component, short minimumDamage)
{
    char message[80];

    if (minimumDamage <
        (short)g_acPlayerComponentDamage[component]) {
        g_acPlayerComponentDamage[component]--;
        sprintf(message, "%s FIXD",
                g_apszComponentNames[component]);
        ShowComponentHitHudMessage(
            message, g_cRedColour, 8);
        return 1;
    } else {
        return 0;
    }
}

/* Function start: 0x4135F1 */
void repair_internal_damage(void)
{
    if (RandomBelowOrEqual(500) < 2) {
        switch (RandomBelowOrEqual(2)) {
        case 0:
            if (g_acPlayerComponentDamage[2] < 4)
                ReportComponentRepaired(2, 1);
            break;
        case 1:
            if (g_acPlayerComponentDamage[0] < 4) {
                if (ReportComponentRepaired(0, 2) != 0)
                    damage_ion_drive(0, -1, 3);
            }
            break;
        case 2:
            if (g_aObjectTypeData[
                    g_acObjectType[0]].damageCapacity - 3 <
                (short)g_acShipDamage[0])
                g_acShipDamage[0]--;
            break;
        }
    }
}

void AddFixedVectors(const FixedVector *left, const FixedVector *right,
                     FixedVector *sum);

/* Function start: 0x4136E2 */
void Create_ship_hit_debris(short obj, short count)
{
    FixedVector offset;
    short debris;
    short piece;

    for (piece = 0; piece < count; piece++) {
        debris = find_vacant_3d_object();
        if (debris == -1)
            return;
        set_objects_data(debris,
            g_aeShipHitDebrisTypes[
                RandomBelowOrEqual(2)], -1, 0);
        g_asObjectCounter[debris] = 40;
        FillFixedVectorWithRandomComponents(10, &offset);
        AddFixedVectors(&g_aShipPosition[obj], &offset,
                        &g_aShipPosition[debris]);
        FillFixedVectorWithRandomComponents(
            6, &g_aShipVelocity[debris]);
        RecordCannedSceneObjectEvent(debris, 0);
    }
}

/* Function start: 0x4137C2 */
void check_next_wave(void)
{
    short i;
    MissionObjective *objectiveData;

    for (i = 0; i < 10; i++) {
        if (g_aeObjectClass[i] >= OBJECT_CLASS_SHIP &&
            g_asShipSide[i] == SIDE_KILRATHI)
            return;
    }
    if (g_nEscapedEnemyCount != 0)
        return;
    for (i = 0; i < (short)g_cMissionObjectiveCount; i++) {
        objectiveData = &g_aMissionObjectives[i];
        if (objectiveData->type == 5 &&
            objectiveData->index == g_nCurrentNavPoint) {
            flag_objective(i, 2);
            return;
        }
    }
}

/* Function start: 0x4138A8 */
void ProcessEnemyWaveCompletion(void)
{
    short obj;

    for (obj = 0; obj < 10; obj++) {
        if (g_aeObjectClass[obj] == OBJECT_CLASS_SHIP &&
            g_asShipSide[obj] == SIDE_KILRATHI)
            return;
    }
    check_next_wave();
    if (g_ucPendingEjectionTransition != 0xff &&
        g_bEjectionWaitForEnemyWave != 0) {
        if (g_bEjectionTriggerImmediately != 0) {
            ejection_sequence(
                g_ucPendingEjectionTransition, 1);
            g_ucPendingEjectionTransition = 0xff;
            g_bEjectionTriggerImmediately = 0;
        } else {
            g_nEjectionSequenceState = 0;
            g_nPendingEjectionSequenceCount++;
        }
    }
    if (set_up_next_wave() == 0) {
        for (obj = 1; obj < 10; obj++) {
            if (g_asShipSide[obj] == SIDE_KILRATHI &&
                g_aeObjectClass[obj] != OBJECT_CLASS_NULL)
                return;
        }
        if (g_ucPendingEjectionTransition != 0xff) {
            g_nEjectionSequenceState = 0;
            g_nPendingEjectionSequenceCount++;
        }
        if (g_bMissionDeathSequencePending != 0 &&
            g_bMissionEjectionPodSpawned != 0) {
            ejection_sequence(
                (short)(g_stCurrentPilotProfile.playerShipType + 300), 1);
            g_bMissionDeathSequencePending = 0;
            g_bMissionEjectionPodSpawned = 0;
        }
    }
}

/* Function start: 0x413A3B */
void Create_explosion_debris(short obj)
{
    FixedVector vector;
    short fragment;
    short i;
    short index;
    short set;

    for (i = 0; i < 5; i++) {
        if (g_aObjectTypeData[
                g_acObjectType[obj]].resourceType ==
            g_aObjectResourceSlots[i].resourceType)
            break;
    }
    remove_object(obj);
    RecordCannedSceneObjectEvent(obj, 1);
    set = RandomBelowOrEqual(3);
    for (index = 0; index < 7; index++) {
        fragment = find_vacant_3d_object();
        if (fragment == -1)
            break;
        set_objects_data(fragment,
                         g_aaeExplosionDebris[set][index], -1, 0);
#ifdef SDL_PORT
        /* A ship can outlive its resource slot: new_sphere_shapes frees the
         * slots the next nav point does not preload.  No slot then holds its
         * wing, and retail takes one from past the five slots. */
        if (g_asObjectType[fragment] == OBJECT_DATA_SHIP_WING && i == 5) {
            remove_object(fragment);
            continue;
        }
#endif
        if (g_asObjectType[fragment] == OBJECT_DATA_SHIP_WING)
            g_apObjectShape[fragment] =
                g_aObjectResourceSlots[i].field_12;
        g_asObjectCounter[fragment] = 40;
        FillFixedVectorWithRandomComponents(50, &vector);
        AddFixedVectors(&g_aShipPosition[obj], &vector,
                        &g_aShipPosition[fragment]);
        FillFixedVectorWithRandomComponents(
            25, &g_aShipVelocity[fragment]);
        divide_vector(&g_aShipVelocity[obj], 0x200, &vector);
        AddFixedVectors(&vector,
                        &g_aShipVelocity[fragment],
                        &g_aShipVelocity[fragment]);
        RecordCannedSceneObjectEvent(fragment, 0);
    }
    for (index = 0; index < 8; index++) {
        fragment = find_vacant_3d_object();
        if (fragment == -1)
            break;
        FillFixedVectorWithRandomComponents(50, &vector);
        AddFixedVectors(&g_aShipPosition[obj], &vector,
                        &g_aShipPosition[fragment]);
        FillFixedVectorWithRandomComponents(
            25, &g_aShipVelocity[fragment]);
        divide_vector(&g_aShipVelocity[obj], 0x200, &vector);
        AddFixedVectors(&vector,
                        &g_aShipVelocity[fragment],
                        &g_aShipVelocity[fragment]);
        g_asObjectScreenAngle[fragment] =
            (short)(RandomBelowOrEqual(3) + 0x10);
        g_aeObjectClass[fragment] = OBJECT_CLASS_DUST;
        g_asObjectType[fragment] = OBJECT_DATA_DEBRIS_DUST;
        g_acObjectType[fragment] =
            (signed char)g_asObjectType[fragment];
        g_asObjectCounter[fragment] = 40;
        RecordCannedSceneObjectEvent(fragment, 0);
    }
    ProcessEnemyWaveCompletion();
}

/* Function start: 0x413D61 */
void affect_mission_score(short pilot, int event, short amount)
{
    short score;

    switch (event) {
    case 0:
        score = amount;
        break;
    case 1:
        score = 7;
        break;
    case 2:
        score = 10;
        break;
    case 3:
    case 4:
        score = 15;
        break;
    case 5:
        score = 25;
        break;
    case 6:
        score = 50;
        break;
    case 7:
        score = 75;
        break;
    case 8:
    case 9:
    case 10:
        score = 25;
        break;
    case 11:
        score = (short)(amount * 2);
        break;
    default:
        score = amount;
        break;
    }
    g_stCurrentPilotProfile.missionScore += score;
}

/* Function start: 0x413E4C */
void score_for_kill(short pilot, short victim)
{
    int event;

    if (g_asShipSide[victim] == SIDE_KILRATHI) {
        /* WC2 renumbered the craft table; the Kilrathi hulls that carry a
         * kill score sit at 48-58 and the retail image names none of them. */
        if (g_aeObjectClass[victim] == OBJECT_CLASS_BASE ||
            g_asObjectType[victim] == 58) {
            event = 7;
        } else {
            switch (g_asObjectType[victim]) {
            case 48:
                event = 1;
                break;
            case 49:
                event = 2;
                break;
            case 50:
                event = 3;
                break;
            case 52:
            case 53:
                event = 5;
                break;
            case 54:
            case 55:
                event = 4;
                break;
            case 56:
            case 57:
                event = 6;
                break;
            }
        }
        affect_mission_score(pilot, event, -1);
    }
}


/* Function start: 0x413F58 */
void analyze_kill(short attacker, short victim)
{
    short hostile;
    short initialShip;

    hostile = g_asShipSide[attacker] !=
            g_asShipSide[victim];
    new_space_music_changes(attacker, victim);
    if (hostile != 0) {
        score_for_kill(attacker, victim);
        if (attacker == 0) {
            g_36_cPilotKillCountValue++;
        } else {
            for (initialShip = 0; initialShip < 8; initialShip++) {
                if (g_13_stMissionHeaderBlock
                        .initialMissionShips[initialShip] ==
                    g_asShipMissionIndex[attacker])
                    g_4_acInitialShipKillNumber[initialShip]++;
            }
        }
    }
}

/* Function start: 0x414025 */
void explosion_shock_wave(short obj, short blastDamage)
{
    short target;
    FixedVector force;
    short range;
    short damage;
    FixedVector delta;

    for (target = 0; target < 10; target++) {
        if (obj != target &&
            g_aeObjectClass[target] >= OBJECT_CLASS_MISSILE) {
            ComputeVectorDelta(&g_aShipPosition[obj],
                               &g_aShipPosition[target], &delta);
            range = MaxShort(0,
                (short)(FixedToShortSaturating(Vector_magnitude(&delta)) -
                        g_asObjectCollisionRadius[target]));
            if ((range > 1000 ||
                (g_aeObjectClass[target] == OBJECT_CLASS_MISSILE &&
                 ((g_asMissileProximityFuseValues[target] + 1) * 100 <
                      range ||
                  g_asShipTactic[target] != 4)))
#ifdef SDL_PORT
                /* SO2.exe 0x454F5B: Mace blasts reach beyond 1000 metres. */
                && g_asObjectType[obj] != OBJECT_TYPE_MACE
#endif
                ) {
                damage = 0;
            } else {
                if (range > 750)
#ifdef SDL_PORT
                    range = g_asObjectType[obj] == OBJECT_TYPE_MACE &&
                            range > 2000 ? 60 : 40;
#else
                    range = 40;
#endif
                else if (range > 500)
                    range = 30;
                else
                    range = find_ratio(0, 500, range, 8, 25);
                damage = (short)((blastDamage / range) / range);
                if (obj != -1 &&
                    g_aeObjectClass[target] >=
                        OBJECT_CLASS_CAPITAL_SHIP &&
                    g_aeObjectClass[obj] == OBJECT_CLASS_MISSILE &&
                    g_asObjectType[obj] != OBJECT_DATA_TORPEDO
#ifdef SDL_PORT
                    && g_asObjectType[obj] != OBJECT_TYPE_MACE
#endif
                    )
                    damage = 0;
                if (target == 0)
                    damage = MaxShort(1, (short)(damage * 3 >> 2));
            }
            if (damage > 1) {
                NormalizeFixedVector(&delta);
                ScaleFixedVector(&delta, (int)damage << 8, &force);
                apply_force_to_objects_center(&force, target);
#ifdef SDL_PORT
                /* SO2.exe 0x45512B: outside 2000 metres the blast pushes
                 * objects but does not damage them. Inside, cap at 4000. */
                if (g_asObjectType[obj] == OBJECT_TYPE_MACE) {
                    if (range < 60)
                        inflict_damage(obj, target,
                                       MinShort(4000, (short)(damage * 8)),
                                       &delta);
                } else
#endif
                if (g_asObjectType[obj] != OBJECT_DATA_TORPEDO) {
                    inflict_damage(obj, target,
                                   MinShort(200, damage), &delta);
                } else {
                    inflict_damage(obj, target,
                                   MinShort(2500, (short)(damage * 4)),
                                   &delta);
                }
            }
        }
    }
}

/* Function start: 0x4142C8 */
short ShipExplosion(short obj)
{
    unsigned short savedScale;
    short explosion;
    short explosionType;

    savedScale = (unsigned short)g_asObjectScale[obj];
    explosion = find_vacant_3d_object();
    if (explosion == -1) {
        if (g_aeObjectClass[obj] >=
            OBJECT_CLASS_CAPITAL_SHIP)
#ifdef SDL_PORT
        {
            /* As in remove_object: a view packet the ship's resource slot
             * preloaded is shared with every ship of its type, so free only
             * one this ship loaded itself. */
            if (g_abObjectShapeOwned[obj] != 0)
                FreePacketAndClear(&g_apObjectShape[obj], 0);
            g_apObjectShape[obj] = 0;
            g_abObjectShapeOwned[obj] = 0;
        }
#else
            FreePacketAndClear(&g_apObjectShape[obj], 0);
#endif
        g_asLoadedShipViewFrame[obj] = -1;
        explosion = obj;
    } else {
        copy_frame(obj, explosion);
        g_aShipPosition[explosion] =
            g_aShipPosition[obj];
        g_aShipVelocity[explosion] =
            g_aShipVelocity[obj];
        g_acObjectOwner[explosion] = (signed char)obj;
    }
    if (g_aeObjectClass[obj] >= OBJECT_CLASS_CAPITAL_SHIP) {
        PlaySfxWaveFileByNumber(0x31, obj, 0);
        explosionType = OBJECT_DATA_EXPLOSION_MEDIUM;
    } else {
        explosionType = OBJECT_DATA_EXPLOSION_LARGE;
    }
    set_objects_data(explosion, explosionType,
                     (short)g_acObjectOwner[explosion], 0);
    g_asObjectScale[explosion] = (short)(
        (unsigned short)g_asObjectScale[explosion] *
        savedScale >> 8);
    RecordCannedSceneObjectEvent(explosion, 0);
    return explosion;
}

short ShipExplosion(short obj);

/* Function start: 0x41444C */
short Explosion(short obj)
{
    short debris;
    short slot;
    short shipIndex;

    debris = obj;
    if (g_aeObjectClass[obj] >= OBJECT_CLASS_SHIP) {
        shipIndex = -1;
        if (g_aeObjectClass[obj] >= OBJECT_CLASS_CAPITAL_SHIP)
            shipIndex = find_ship_index(obj);
        if (obj < 10 &&
            (g_asPilotLevel[obj] == 5 ||
             (shipIndex != -1 &&
              g_asShipMissionParameter[obj] == shipIndex) ||
             g_aeObjectClass[obj] >= OBJECT_CLASS_CAPITAL_SHIP ||
             RandomBelow(100) < 40)) {
            if (g_asShipSide[obj] == 1)
                send_message(obj, 30);
            else
                send_message(obj, 8);
        }
        for (slot = 0; slot < 8; slot++) {
            if (g_13_stMissionHeaderBlock.initialMissionShips[slot] ==
                g_asShipMissionIndex[obj]) {
                g_acMissionShipStatusFlagsBytes[slot] |= 1;
                if (g_nYourWingman == obj)
                    g_nYourWingman = -1;
            }
        }
        set_special(obj, SPECIAL_MANEUVER_UNKNOWN_9);
        if (g_aObjectTypeData[
                OBJECT_DATA_EXPLOSION_LARGE].shapeSet != 0)
            g_asObjectCounter[obj] = 8;
        else
            g_asObjectCounter[obj] = 4;
        if (g_aObjectTypeData[
                OBJECT_DATA_EXPLOSION_LARGE].shapeSet != 0)
            g_7_asShipBurstStageCountdown[obj] = 8;
        else
            g_7_asShipBurstStageCountdown[obj] = 4;
        g_aMissionShips[
            g_asShipMissionIndex[obj]].state = 3;
        if (g_aeObjectClass[obj] >= OBJECT_CLASS_CAPITAL_SHIP) {
            for (slot = 0; slot < 4; slot++)
                onboard_explosion(obj);
            g_asObjectCounter[obj] = (short)(
                (g_aObjectTypeData[
                     g_acObjectType[obj]].damageCapacity >> 2) +
                (g_aObjectTypeData[
                     OBJECT_DATA_EXPLOSION_LARGE].shapeSet != 0 ? 8 : 4));
            g_7_asShipBurstStageCountdown[obj] = (short)(
                (g_aObjectTypeData[
                     OBJECT_DATA_EXPLOSION_LARGE].shapeSet != 0 ?
                     8 : 4) * 2);
            if (g_7_asShipBurstStageCountdown[obj] <
                g_9_nScreenMessageTimeNumber) {
                g_7_asShipBurstStageCountdown[obj] =
                    g_9_nScreenMessageTimeNumber;
            }
        } else {
            debris = ShipExplosion(obj);
            explosion_shock_wave(obj,
                g_aObjectTypeData[
                    g_acObjectType[obj]].explosionDamage);
        }
    } else {
        if (g_aeObjectClass[debris] == OBJECT_CLASS_ASTEROID)
            g_asObjectScale[debris] = 0x380;
        explosion_shock_wave(obj,
            g_aObjectTypeData[
                g_acObjectType[obj]].explosionDamage);
        if (g_asObjectType[obj] == OBJECT_DATA_TORPEDO) {
            debris = obj;
            set_objects_data(debris, OBJECT_DATA_EXPLOSION_LARGE,
                             g_acObjectOwner[obj], 0);
#ifdef SDL_PORT
        } else if (g_asObjectType[obj] == OBJECT_TYPE_MACE) {
            /* SO2.exe 0x4556B0: retain the missile owner for kill credit. */
            debris = obj;
            set_objects_data(debris, OBJECT_TYPE_NUCLEAR_EXPLOSION,
                             g_acObjectOwner[obj], 0);
#endif
        } else {
            debris = obj;
            set_objects_data(debris, OBJECT_DATA_EXPLOSION_SMALL,
                             g_acObjectOwner[obj], 0);
        }
        RecordCannedSceneObjectEvent(debris, 0);
    }
    if (g_asObjectScreenX[obj] != -32767)
        PlaySfxWaveFileByNumber(4, obj, 0);
    return debris;
}

/* Function start: 0x414835 */
short explode(short attacker, short victim)
{
    short result;
    short object;

    if (victim < 10)
        g_anShipCloakState[victim] = 2;
#ifdef SDL_PORT
    if (g_nSdlExplosionDepth == 0) {
#else
    if (g_bExplosionTraversalIdle != 0) {
#endif
        for (object = 0; object < SPACE_OBJECT_COUNT; object++)
            g_10_abExplosionEntityDoneBytes[object] = 0;
    }
#ifdef SDL_PORT
    g_nSdlExplosionDepth++;
    if (g_10_abExplosionEntityDoneBytes[victim] != 0 ||
        g_aeSpecialManeuver[victim] == SPECIAL_MANEUVER_UNKNOWN_9) {
        g_nSdlExplosionDepth--;
        return 1;
    }
    g_10_abExplosionEntityDoneBytes[victim] = 1;
#else
    g_bExplosionTraversalIdle = 0;
    if (g_10_abExplosionEntityDoneBytes[victim] != 0)
        return 1;
    else
        g_10_abExplosionEntityDoneBytes[victim] = 1;
#endif
    result = ResolveObjectDestruction(attacker, victim);
#ifdef SDL_PORT
    g_nSdlExplosionDepth--;
#else
    g_bExplosionTraversalIdle = 1;
#endif
    return result;
}

/* Function start: 0x4148F5 */
short ResolveObjectDestruction(short attacker, short victim)
{
    short objective;

    if (victim < 10 && g_asPilotLevel[victim] == 5) {
        if (g_abPilotEjectionAttempted[victim] == 0) {
            g_abPilotEjectionAttempted[victim]++;
            g_acShipStress[victim] = 120;
            if (g_asShipSide[victim] == SIDE_IMPERIAL)
                reset_maneuver(victim, MANEUVER_OUTA_HERE);
            g_acShipDamage[victim] =
                (signed char)(g_acShipDamage[victim] / 2);
            if (g_asShipSide[victim] == SIDE_KILRATHI)
                send_message(victim, 0x1d);
            return 0;
        } else if (RandomBelowOrEqual(1) == 0)
            return 0;
    }
    if (g_aeSpecialManeuver[victim] ==
            SPECIAL_MANEUVER_UNKNOWN_9 &&
        g_aeObjectClass[victim] >= OBJECT_CLASS_SHIP)
        return 0;
    if (victim == 0) {
        if (g_bPlayerDamageEnabled == 0)
            return 0;
        else {
            g_8_bUserDestroyedFlag = 1;
            if (g_bDeathSequenceActive == 0)
                g_nArcadeState = 4;
            return 1;
        }
    } else if (victim == g_nExternalViewShip)
        g_nExternalViewShip = -1;
#ifdef SDL_PORT
    /* With no attacker, WC2 reads the zero-filled byte at 0x495207 just
     * before the owner table and consequently attributes the destruction to
     * object 0.  Preserve that result without crossing the native redzone. */
    if (attacker == -1)
        attacker = 0;
    else
        attacker = g_acObjectOwner[attacker];
#else
    if (g_acObjectOwner[attacker] == -1) {
    } else {
        attacker = g_acObjectOwner[attacker];
    }
#endif
    if (attacker != -1 &&
        g_aeObjectClass[victim] >= OBJECT_CLASS_SHIP) {
        analyze_kill(attacker, victim);
        for (objective = 0; objective < MISSION_OBJECTIVE_COUNT;
             objective++) {
            if (g_aMissionObjectives[objective].type == 4 &&
                g_aMissionObjectives[objective].index ==
                    g_asShipMissionIndex[victim]) {
                g_aMissionObjectives[objective].field_8 =
                    (unsigned char)g_asShipMissionIndex[attacker];
            }
        }
    }
    if (victim < 10 && g_asShipSide[victim] != SIDE_KILRATHI &&
        g_abShipEjectionSequenceEnabled[victim] != 0) {
        if (g_aeObjectClass[victim] >= OBJECT_CLASS_CAPITAL_SHIP)
            return 0;
        else if (g_aeObjectClass[victim] == OBJECT_CLASS_SHIP) {
            BeginShipDestructionSequence(victim);
            return 1;
        }
    }
    Explosion(victim);
    return 1;
}

short SpawnFlakBurst(short obj, short range, FixedVector *hardpoint,
                     short burstMode);
void fire_flack(short owner, short explosion, short range,
                FixedVector *aim, short burstMode);
short hemisphere(FixedVector *target, FixedVector *parent,
                 FixedVector *hardpoint);

/* Function start: 0x414BE9 */
int send_at_point(short obj, FixedVector *point, short speed)
{
    ComputeVectorDelta(&g_aShipPosition[obj], point,
                       &g_aShipVelocity[obj]);
    SetVectorFixedPoint((unsigned int *)&g_aShipVelocity[obj],
                        speed);
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x414C39 */
short find_child_object(short parent, short objectClass)
{
    short obj;

    for (obj = 0; obj < SPACE_OBJECT_COUNT; obj++) {
        if ((short)g_acObjectOwner[obj] == parent &&
            g_aeObjectClass[obj] == objectClass)
            return obj;
    }
    return -1;
}

/* Function start: 0x414CA9 */
short find_child_ship(short parent, short objectClass,
                      short target)
{
    short obj;

    for (obj = 0; obj < 10; obj++) {
        if ((short)g_acObjectOwner[obj] == parent &&
            g_aeObjectClass[obj] == objectClass &&
            (target == -1 ||
             g_acShipTarget[obj] == target))
            return obj;
    }
    return -1;
}

/* Function start: 0x414D3D */
int launch_object(short parent, short child, FixedVector direction,
                  short speed)
{
    NormalizeFixedVector(&direction);
    vector_component_in_dir(&g_aShipVelocity[parent],
                            &direction,
                            &g_aShipVelocity[child]);
    ScaleFixedVector(&direction, (int)speed << 8, &direction);
    AddFixedVectors(&direction, &g_aShipVelocity[child],
                    &g_aShipVelocity[child]);
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x414DC0 */
void fire(short obj, short target)
{
    short missileOk;
    short firedObject;
    short shouldFire;
    short ready;
    short distance;
    short weaponRange;
    short reachable;
    ObjectTypeData *stats;
    short slot;
    short weaponVelocity;
    short other;
    short type;
    FixedVector direction;
    short nearestRange;

    ready = g_asObjectCounter[obj] <= 0;
    if (g_asShipFriendlyFireCooldown[obj] > 0)
        return;
    if (CountShipProjectiles(obj) > g_nAdaptiveDifficulty)
        return;

    distance = g_nTargetRange;
    if (RandomBelowOrEqual(0x13) == 0 &&
        RandomBelowOrEqual(7000) > distance)
        missileOk = 1;
    else
        missileOk = 0;
    if (missileOk != 0 &&
        g_aeObjectClass[target] >= OBJECT_CLASS_CAPITAL_SHIP &&
        g_nYourWingman == obj &&
        (unsigned short)RandomInRange(0, 100) > 10)
        missileOk = 0;
    if (missileOk != 0 &&
        find_child_ship(obj, OBJECT_CLASS_MISSILE, target) != -1)
        missileOk = 0;
    if (g_bAiMissileFiringEnabled == 0)
        missileOk = 0;

    slot = 0;
    for (; slot < (signed char)g_aShipWeapons[obj][0];
         slot++) {
        type = ((ShipWeaponSlot *)&g_aShipWeapons[obj][1])[
                         slot].weaponType;
        stats = &g_aObjectTypeData[type];
        type = ((ShipWeaponSlot *)&g_aShipWeapons[obj][1])[
                         slot].type;
        weaponVelocity = stats->maximumVelocity;
        weaponRange =
            (short)(*(int *)&stats->lifetime * weaponVelocity);
        reachable = weaponRange > distance;
        shouldFire = 0;

        switch (stats->objectClass) {
        case OBJECT_CLASS_PROJECTILE:
            if (CanShipWeaponDamageTarget(obj, target) != 0 &&
                ready != 0 && reachable != 0) {
                switch (type) {
                case 7:
                    shouldFire = g_nFacingToTarget > 70;
                    break;
                case 10:
                    shouldFire = g_nFacingToTarget > 80;
                    break;
                case 8:
                    shouldFire = g_nFacingToTarget > 80;
                    break;
                case 9:
                    shouldFire = g_nFacingToTarget > 85;
                    break;
                case 13:
                    shouldFire = g_nFacingToTarget > 55;
                    break;
                case 12:
                    shouldFire = g_nFacingToTarget > 10;
                    break;
                }
            }
            if (shouldFire != 0)
                ((ShipWeaponSlot *)&g_aShipWeapons[obj][1])[
                    slot].disabled = 0;
            else
                ((ShipWeaponSlot *)&g_aShipWeapons[obj][1])[
                    slot].disabled = 1;
            if (shouldFire != 0) {
                firedObject = fire_weapon(obj, slot);
                if (g_anShipCloakState[obj] == 1)
                    ResetShipCloakState(obj);
                if (type == 12) {
                    ComputeVectorDelta(
                        &g_aShipPosition[firedObject],
                        &g_aShipPosition[target], &direction);
                    launch_object(obj, firedObject, direction,
                                  real_velocity(firedObject));
                }
            }
            break;

        case OBJECT_CLASS_MISSILE:
            if (missileOk != 0 && reachable != 0) {
                switch (type) {
                case 0x0f:
                    shouldFire = g_nFacingToTarget > 0x61;
                    if (CanShipWeaponDamageTarget(obj, target) == 0)
                        shouldFire = 0;
                    if (shouldFire != 0 &&
                        g_asShipSide[obj] == SIDE_KILRATHI) {
                        nearestRange = g_nTargetRange;
                        other = 1;
                        for (; other < 10; other++) {
                            if (g_aeObjectClass[other] ==
                                    OBJECT_CLASS_SHIP &&
                                g_asShipSide[other] ==
                                    SIDE_KILRATHI) {
                                get_facing_range_from_object(obj, other);
                                if (g_nFacingToTarget > 0x5f &&
                                    g_nTargetRange < nearestRange) {
                                    shouldFire = 0;
                                    break;
                                }
                            }
                        }
                        get_facing_range_from_object(obj, target);
                    }
                    break;
                case 0x10:
                    shouldFire = g_nFacingToTarget > 0x28 &&
                                 g_nTargetFacing < -0x3c;
                    if (CanShipWeaponDamageTarget(obj, target) == 0)
                        shouldFire = 0;
                    break;
                case 0x12:
                    shouldFire = g_nFacingToTarget > 0x28;
                    if (CanShipWeaponDamageTarget(obj, target) == 0)
                        shouldFire = 0;
                    break;
                case 0x11:
                    shouldFire = 1;
                    if (CanShipWeaponDamageTarget(obj, target) == 0)
                        shouldFire = 0;
                    break;
                case 0x13:
                    if (IsCapitalShipObject(target) != 0 &&
                        g_nFacingToTarget > 0x50)
                        shouldFire = 1;
                    else
                        shouldFire = 0;
                    break;
                }
            }
            if (shouldFire != 0) {
                if (g_anShipCloakState[obj] == 1)
                    ResetShipCloakState(obj);
                fire_weapon(obj, slot);
                missileOk = 0;
            }
            break;
        }
    }
}

/* Function start: 0x415508 */
short hemisphere(FixedVector *target, FixedVector *parent,
                 FixedVector *hardpoint)
{
    FixedVector toHardpoint;
    FixedVector toTarget;

    ComputeVectorDelta(hardpoint, parent, &toHardpoint);
    ComputeVectorDelta(target, parent, &toTarget);
    return vector_angle(toHardpoint, toTarget);
}

/* Function start: 0x415563 */
void fire_flack(short owner, short explosion, short range,
                FixedVector *aim, short burstMode)
{
    short projectileVelocity;
    short lifetime;

    projectileVelocity = g_aObjectTypeData[0x0c].maximumVelocity;
    set_objects_data(explosion, 0x0c, owner, 0);
    if (burstMode == 0) {
        lifetime = (short)(range / projectileVelocity - 5 -
                           RandomBelowOrEqual(8));
        g_asObjectCounter[explosion] =
            MinShort(55, MaxShort(5, lifetime));
    } else {
        projectileVelocity = 220;
        g_asObjectCounter[explosion] =
            (short)(range / projectileVelocity);
    }
    send_at_point(explosion, aim, projectileVelocity);
    RecordCannedSceneObjectEvent(explosion, 0);
}

/* Function start: 0x415625 */
short RandomSign(short v)
{
    if (RandomBelowOrEqual(1) != 0)
        return v;
    else
        return -v;
}

/* Function start: 0x41565E */
int RandomFixedAimComponent(short radius, short speed, short maximum)
{
    short aim;

    aim = RandomSign(MinShort(
        maximum, (short)(RandomBelowOrEqual(radius) + speed)));
#ifdef SDL_PORT
    /* SHL supplies the fixed-point scale in retail, including for a negative
     * random component; multiplication has the same value without C shift UB. */
    return (int)aim * 256;
#else
    return aim << 8;
#endif
}

/* Function start: 0x4156A7 */
short SpawnFlakBurst(short obj, short range, FixedVector *hardpoint,
                     short burstMode)
{
    FixedVector randomAim;
    FixedVector burstPoint;
    FixedVector targetMotion;
    short target;
    short burstObject;
    short aimRadius;
    short velocity;
    short rollPercent;
    short predictionSteps;

    target = g_acShipTarget[obj];
    burstObject = new_object(0x24, obj);
    if (burstObject != -1) {
        aimRadius = MaxShort(400, (short)(range >> 3));
        velocity = (short)(real_velocity(target) << 3);
        zero_vector(&randomAim);
        if (burstMode == 0) {
            randomAim.x =
                RandomFixedAimComponent(aimRadius, velocity, 1000);
            randomAim.y =
                RandomFixedAimComponent(aimRadius, velocity, 1000);
            randomAim.z =
                RandomFixedAimComponent(aimRadius, velocity, 1000);
        }
        AddFixedVectors(&g_aShipPosition[target], &randomAim,
                        &burstPoint);
        rollPercent = RandomBelowOrEqual(100);
        if (burstMode != 0) {
            if (g_asObjectCounter[obj] <= 0) {
                g_aShipPosition[burstObject] = *hardpoint;
                predictionSteps = (short)(range / 70);
                if (predictionSteps < 0)
                    predictionSteps = 0;
                if (predictionSteps > 20)
                    predictionSteps = 20;
                ScaleFixedVector(&g_aShipVelocity[target],
                                 predictionSteps << 8, &targetMotion);
                AddFixedVectors(&targetMotion, &burstPoint, &burstPoint);
                fire_flack(obj, burstObject, range, &burstPoint, burstMode);
                g_asObjectCounter[obj] =
                    (short)(RandomBelow(10) + 7);
            }
        } else if ((g_asObjectCounter[obj] <= 0 && rollPercent < 40) ||
                   rollPercent < 8) {
            g_aShipPosition[burstObject] = *hardpoint;
            fire_flack(obj, burstObject, range, &burstPoint, burstMode);
            g_asObjectCounter[obj] =
                (short)(RandomBelow(10) + 7);
        } else if (g_asObjectType[
                       (signed char)g_acShipTarget[obj]] != 0x13) {
            g_aShipPosition[burstObject] = burstPoint;
            explosion_shock_wave(
                burstObject,
                g_aObjectTypeData[0x0c].explosionDamage);
        }
    }
    return burstObject;
}

/* Function start: 0x41596F */
short HasShipActiveMissile(short obj)
{
    short other;

    for (other = 0; other < 10; other++) {
        if (g_aeObjectClass[other] == OBJECT_CLASS_MISSILE &&
            g_acObjectOwner[other] == obj)
            return 1;
    }
    return 0;
}

/* Function start: 0x4159DB */
short fire_turrets(short obj)
{
    short mounts;
    FixedVector mountOrigin;
    short startTarget;
    short targetHemisphere;
    short turret;
    short targetRange;
    short pick;
    short hostiles;
    short candidate;
    ShipWeaponSlot *fitting;
    short lockedShip;

    mounts = 0;
    targetRange = 10000;
    if (g_asShipSide[obj] == SIDE_IMPERIAL)
        targetRange = 5000;
    hostiles = build_target_list(obj, targetRange);
    for (turret = 0;
         turret < (signed char)g_aShipWeapons[obj][0];
#ifdef SDL_PORT
         /* WC2 walks the fitting pointer forward here before anything sets it -
          * this loop indexes the array directly and the loop below starts the
          * pointer over - and advancing an indeterminate pointer is undefined
          * behaviour the sanitizers catch.  Leave it alone on the port. */
         turret++) {
#else
         turret++, fitting++) {
#endif
        if (((ShipWeaponSlot *)&g_aShipWeapons[obj][1])[
                turret].type == 0x0c)
            mounts++;
    }

    fitting = (ShipWeaponSlot *)&g_aShipWeapons[obj][1];
    for (turret = 0;
         turret < (signed char)g_aShipWeapons[obj][0];
         turret++, fitting++) {
        if (fitting->type == 0x0d || fitting->type == 0x0e)
            continue;
        if ((g_asShipSide[obj] == SIDE_KILRATHI &&
             (unsigned short)RandomInRange(0, 200) <
                 GetAdaptiveTurnRate()) ||
            (g_asShipSide[obj] == SIDE_IMPERIAL &&
             RandomBelowOrEqual(2) == 0)) {
            position_child(obj, turret, &mountOrigin);
            if (fitting->type == 0x0c) {
                for (candidate = 1; candidate < 10; candidate++) {
                    if (g_aeObjectClass[candidate] !=
                            OBJECT_CLASS_NULL &&
                        g_asObjectType[candidate] == 0x13 &&
                        g_acShipTarget[candidate] == obj) {
                        targetHemisphere = hemisphere(
                            &g_aShipPosition[candidate],
                            &g_aShipPosition[obj], &mountOrigin);
                        if (g_asMissileProximityFuseValues[candidate] != 0 &&
                            RandomBelowOrEqual(
                                (short)(mounts * 4 - 1)) == 0) {
                            g_acShipTarget[obj] =
                                (signed char)candidate;
                            g_asObjectCounter[obj] = 0;
                            SpawnFlakBurst(
                                obj, distance_from_object(obj, candidate),
                                &mountOrigin, 1);
                        }
                    }
                }
            }
            if (hostiles != 0) {
                startTarget = RandomBelowOrEqual(
                    (short)(hostiles - 1));
                pick = startTarget;
                do {
                    lockedShip = (short)g_acShipList[pick];
                    targetHemisphere = hemisphere(
                        &g_aShipPosition[lockedShip],
                        &g_aShipPosition[obj], &mountOrigin);
                    if (fitting->type != 0x0c &&
                        g_aeObjectClass[lockedShip] >=
                            OBJECT_CLASS_SHIP) {
                        if (targetHemisphere > 60 &&
                            RandomBelowOrEqual(
                                (short)(200 - GetAdaptiveTurnRate())) == 0 &&
                            HasShipActiveMissile(obj) == 0) {
                            if (fitting->type == 0x13) {
                                if (IsCapitalShipObject(lockedShip) != 0) {
                                    g_acShipTarget[obj] =
                                        (signed char)lockedShip;
                                    fire_weapon(obj, turret);
                                }
                            } else {
                                g_acShipTarget[obj] =
                                    (signed char)lockedShip;
                                fire_weapon(obj, turret);
                            }
                        }
                        break;
                    }
                    if (RandomBelowOrEqual(2) == 0) {
                        g_acShipTarget[obj] =
                            (signed char)lockedShip;
                        SpawnFlakBurst(
                            obj, g_asShipListRange[pick],
                            &mountOrigin, 0);
                        break;
                    }
                    pick++;
                    if (pick > hostiles - 1)
                        pick = 0;
                } while (pick != startTarget);
            }
        }
    }
    return 1;
}

void ScaleFixedVector(FixedVector *vector, int scale, FixedVector *result);
void copy_frame(short source, short destination);

/* Function start: 0x415E2C */
short fire_weapon(short obj, short weapon)
{
    short triggerDistance;
    short leadRange;
    int interceptScale;
    short projectileSpeed;
    FixedVector travel;
    /* Retail reserves a travel here that nothing uses. */
    FixedVector unusedVector;
    FixedVector mountOffset;
    short typeCode;
    short weaponClass;
    short targetRange;
    short missile;
    ObjectTypeData *weaponData;
    short weaponDataIndex;
    short target;

    projectileSpeed = 10;
    typeCode = ((ShipWeaponSlot *)&g_aShipWeapons[obj][1])[
                     weapon].type;
    weaponDataIndex =
        ((ShipWeaponSlot *)&g_aShipWeapons[obj][1])[
            weapon].weaponType;
#ifdef SDL_PORT
    /* SO2.exe 0x456DBB indexes the mapped loadout field, not type 64. */
    weaponClass = g_aObjectTypeData[weaponDataIndex].objectClass;
#else
    weaponClass = g_aObjectTypeData[typeCode].objectClass;
#endif
    if (typeCode == 0x0c) {
        typeCode = 7;
        weaponClass = 8;
    }
    if (weaponClass == OBJECT_CLASS_MISSILE) {
        if (get_ship_slot() == -1) {
            for (target = 1; target < 10; target++) {
                if (g_aeObjectClass[target] ==
                        OBJECT_CLASS_MISSILE &&
                    g_asObjectType[target] == 0x14 &&
                    g_acObjectOwner[target] == obj)
                    explode(target, target);
            }
        }
        missile = initialize_ship(typeCode, obj, 0);
    } else {
        missile = new_object(typeCode, obj);
    }
    if (missile != -1) {
        weaponData = &g_aObjectTypeData[weaponDataIndex];
        copy_frame(obj, missile);
        if (weaponClass == OBJECT_CLASS_PROJECTILE) {
            g_asObjectDamage[missile] =
                weaponData->damageCapacity;
            projectileSpeed = g_aObjectTypeData[
                g_acObjectType[missile]].maximumVelocity;
            g_asShipWeaponEnergy[obj] =
                (short)(g_asShipWeaponEnergy[obj] -
                        weaponData->animationDelay);
        }
        child_object(weapon, missile, obj);
        g_asObjectCounter[missile] =
            *(int *)&g_aObjectTypeData[
                weaponDataIndex].lifetime;
        vector_component_in_dir(&g_aShipVelocity[obj],
                                &g_aShipForwardVector[missile],
                                &g_aShipVelocity[missile]);
        if (weaponClass == OBJECT_CLASS_PROJECTILE) {
            if (obj == 0 && g_bTargetLockActive != 0 &&
                g_bMissileLockAcquired != 0) {
                targetRange = distance_from_object(
                    0, g_acShipTarget[0]);
                leadRange = (short)(targetRange + 100);
            } else {
                leadRange = (short)((*(int *)&weaponData->lifetime + 5) *
                                       weaponData->maximumVelocity);
            }
            ScaleFixedVector(&g_aShipForwardVector[obj],
                             leadRange << 8, &travel);
            AddFixedVectors(&g_aShipPosition[obj], &travel,
                            &travel);
            point_at(missile, travel);
        }
        ScaleFixedVector(&g_aShipForwardVector[missile],
                         (int)projectileSpeed << 8, &travel);
        AddFixedVectors(&travel, &g_aShipVelocity[missile],
                        &g_aShipVelocity[missile]);
        RecordCannedSceneObjectEvent(missile, 0);
        if (weaponClass == OBJECT_CLASS_MISSILE) {
            ScaleFixedVector(&g_aShipUpVector[obj], 0xa00,
                             &travel);
            AddFixedVectors(&travel, &g_aShipVelocity[missile],
                            &g_aShipVelocity[missile]);
            if (obj == 0 && typeCode != 0x14)
                RemovePlayerReleaseWeapon(weapon);
            else
                remove_weapon(obj, weapon);
            g_aeSpecialManeuver[missile] = -1;
            g_asShipManeuver[missile] =
                g_aeSpecialManeuver[missile];
            g_asShipTactic[missile] = 1;
            g_asPilotLevel[missile] = -1;
            g_abShipEjectionSequenceEnabled[missile] = 0;
            g_asObjectCounter[missile] = 5;
            g_acObjectCollisionGraceTicks[missile] = 20;
            switch (typeCode) {
#ifdef SDL_PORT
            case OBJECT_TYPE_MACE:
                /* SO2.exe 0x4571CC: unguided, five-tick launch delay. */
                steady_object(missile);
                g_acShipTarget[missile] = -1;
                g_anShipSpeed[missile] =
                    get_ship_max_velocity(missile) << 8;
                break;
#endif
            case 0x0f:
                steady_object(missile);
                g_acShipTarget[missile] =
                    g_acShipTarget[obj];
                g_asObjectCounter[missile] = 1;
                g_anShipSpeed[missile] =
                    get_ship_max_velocity(missile) << 8;
                if (g_acShipTarget[missile] != -1) {
                    ComputeVectorDelta(&g_aShipPosition[obj],
                        &g_aShipPosition[
                            g_acShipTarget[missile]], &travel);
                    interceptScale =
                        Vector_magnitude(&travel) /
                        get_ship_max_velocity(missile);
                    ScaleFixedVector(&g_aShipVelocity[
                        g_acShipTarget[missile]],
                        interceptScale, &travel);
                    AddFixedVectors(&g_aShipPosition[
                        g_acShipTarget[missile]], &travel,
                        &travel);
                    point_at(missile, travel);
                }
                break;
            case 0x10:
            case 0x12:
                g_acShipTarget[missile] =
                    g_acShipTarget[obj];
                break;
            case 0x13:
                g_acShipTarget[missile] =
                    g_acShipTarget[obj];
#ifdef SDL_PORT
                /* View 4 is the player's torpedo-lock view.  Retail tests
                 * only the view, so a torpedo any ship launches meanwhile
                 * takes the player's saved lock, or -1 when none is saved.
                 * Keep that lock for the player's own shot. */
                if (obj == 0 && g_nCurrentView == 4)
#else
                if (g_nCurrentView == 4)
#endif
                    g_acShipTarget[missile] =
                        (signed char)g_nSavedPlayerTarget;
#ifdef SDL_PORT
                if (g_acShipTarget[missile] == -1) {
                    /* Retail measures to "object -1": the scratch view
                     * object's forward vector precedes the position table,
                     * and zero padding precedes the radius table. */
                    triggerDistance = (short)(distance_from_point(
                        obj, &g_aShipForwardVector[SCRATCH_VIEW_OBJECT]) /
                        1000);
                } else
#endif
                triggerDistance = (short)(distance_from_object(
                    obj, g_acShipTarget[missile]) / 1000);
                if (triggerDistance > 9 ||
                    (unsigned short)RandomInRange(0, 15) <
                        triggerDistance)
                    g_asMissileProximityFuseValues[missile] = 1;
                else
                    g_asMissileProximityFuseValues[missile] = 0;
                break;
            case 0x11:
                g_asObjectCounter[missile] = 15;
                g_acShipTarget[missile] = -1;
                break;
            case 0x14:
                g_asObjectCounter[missile] = 120;
                g_acShipTarget[missile] = -1;
                ScaleFixedVector(&g_aShipForwardVector[obj],
                                 0xa00, &mountOffset);
                SubtractFixedVectors(&g_aShipVelocity[obj],
                                     &mountOffset,
                                     &g_aShipVelocity[missile]);
                break;
            }
        }
        if (obj == 0) {
            if (weaponClass == OBJECT_CLASS_PROJECTILE) {
                g_asObjectCounter[obj] =
                    g_acGunRefireDelay[
                        typeCode - 7];
                if (g_asObjectType[0] == 0x33)
                    g_asObjectCounter[obj] <<= 1;
            }
        } else if (obj == g_nYourWingman) {
            g_asObjectCounter[obj] =
                (short)(10 - (g_nAdaptiveDifficulty >> 1));
        } else {
            g_asObjectCounter[obj] =
                (short)(14 - g_nAdaptiveDifficulty);
        }
#ifdef SDL_PORT
        if (obj == 0)
            SdlQueueJoystickWeaponRumble(typeCode);
#endif
        switch (typeCode) {
        case 7:
        case 8:
        case 0x0d:
            PlaySfxWaveFileByNumber(8, missile, 0);
            break;
        case 9:
        case 0x0c:
        case 0x14:
            PlaySfxWaveFileByNumber(5, missile, 0);
            break;
        case 0x0a:
            PlaySfxWaveFileByNumber(0x27, missile, 0);
            break;
        case 0x0f:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
#ifdef SDL_PORT
        case OBJECT_TYPE_MACE:
#endif
            PlaySfxWaveFileByNumber(1, missile, 0);
            break;
        }
    }
    return missile;
}

/* Function start: 0x416690 */
short find_weapon(short obj, short weaponType)
{
    short weapon;

    for (weapon = 0;
         weapon < (signed char)g_aShipWeapons[obj][0];
         weapon++) {
        if (((ShipWeaponSlot *)&g_aShipWeapons[obj][1])[weapon].type ==
            weaponType)
            return weapon;
    }
    return -1;
}

/* Function start: 0x41670E */
short fire_missile(short ship)
{
    short weapon;
    ShipWeaponSlot *index;

    index = (ShipWeaponSlot *)&g_aShipWeapons[ship][1];
    for (weapon = 0;
         weapon < (signed char)g_aShipWeapons[ship][0];
         weapon++, index++) {
        if (g_aObjectTypeData[index->weaponType].objectClass ==
                OBJECT_CLASS_MISSILE) {
            if (ship != 0) {
                if (index->type != 0x13 ||
                    IsCapitalShipObject(g_acShipTarget[ship]) != 0)
                    return fire_weapon(ship, weapon);
            } else if (index->disabled == 0) {
                if ((index->type == 0x10 || index->type == 0x12 ||
                     index->type == 0x13) &&
                    g_nTargetLockCountdown != 0) {
                    if (get_mode(0) == 1)
                        ShowComponentHitHudMessage(
                            _("Need Lock"),
                            g_cYellowColour, 3);
                    return -1;
                }
                return fire_weapon(ship, weapon);
            }
        }
    }
    return -1;
}

/* Function start: 0x41687F */
void fire_fixed_projectile_weapon(short obj)
{
    ShipWeaponSlot *index;
    short weapon;

    index = (ShipWeaponSlot *)&g_aShipWeapons[obj][1];
    for (weapon = 0;
         weapon < (signed char)g_aShipWeapons[obj][0];
         weapon++, index++) {
#ifdef SDL_PORT
        SdlTracef("[index] obj=%d w=%d type=%d wtype=%d class=%d dis=%d\n",
                     (int)obj, (int)weapon, (int)index->type,
                     (int)index->weaponType,
                     (int)g_aObjectTypeData[
                         index->weaponType].objectClass,
                     (int)index->disabled);
#endif
        if (g_aObjectTypeData[index->weaponType].objectClass ==
                OBJECT_CLASS_PROJECTILE &&
            index->type != 0x0b && index->disabled == 0) {
            if (fire_weapon(obj, weapon) == -1)
                break;
        }
    }
}

/* Function start: 0x41693A */
void fire_afterburner(short obj, short time)
{
    if (g_asObjectType[obj] == 0x33)
        return;
    /* CL 4.1 folds these complements after ordering the operands. Keep
     * this value-preserving form for the retail limit-first comparison. */
    if (~~(get_ship_max_velocity(obj) * 0x500) >
            Vector_magnitude(&g_aShipVelocity[obj])) {
        set_special(obj, SPECIAL_MANEUVER_AFTERBURNER);
        if (g_aeSpecialManeuver[obj] !=
            SPECIAL_MANEUVER_AFTERBURNER)
            time = 0;
        g_asShipAfterburnerTimer[obj] = time;
    }
}

/* Function start: 0x4169CC */
void fire_super_brake(short ship)
{
    g_asShipAfterburnerTimer[ship] = 10;
    set_special(ship, SPECIAL_MANEUVER_SUPER_BRAKE);
}

/* Function start: 0x4169F3 */
short flip_angle(short ship, short angle)
{
    if ((g_asObjectFlip[ship] & 0x10) != 0)
        angle = (short)(180 - angle);
    if ((g_asObjectFlip[ship] & 0x20) != 0)
        angle = (short)-angle;
    angle += g_asObjectScreenAngle[ship];
    angle %= 360;
    if (angle < 0)
        angle += 360;
    return angle;
}

/* Function start: 0x416A87 */
void place_exhaust_on_ships(void)
{
    short *exhaustTable;
    short shapeFrame;
    short scale;
    short object;
    short ship;
    short viewFrame;
    short tableOffset;

    for (ship = 0; ship < 10; ship++) {
        if (g_aeObjectClass[ship] >= OBJECT_CLASS_MISSILE &&
            g_anShipSpeed[ship] != 0 &&
            g_asObjectType[ship] != 0x14 &&
            g_aeSpecialManeuver[ship] !=
                SPECIAL_MANEUVER_UNKNOWN_9 &&
            g_aeSpecialManeuver[ship] !=
                SPECIAL_MANEUVER_KILL_ENGINES) {
            if (g_anShipCloakState[ship] != 1) {
                if (ship == 0 && g_bJumpSequenceActive != 0)
                    continue;
                if (g_asObjectScreenX[ship] != (short)0x8001) {
                    exhaustTable =
                        (short *)g_apObjectExhaustShape[ship];
                    if (exhaustTable != 0) {
                        viewFrame = g_asObjectViewFrame[ship];
                        /* These ships load each view as a separate,
                         * single-frame packet, so the loaded view's exhaust
                         * records are used rather than frame zero within
                         * that packet. */
                        if (g_aeObjectClass[ship] >=
                                OBJECT_CLASS_CAPITAL_SHIP ||
                            g_asObjectType[ship] == 0x33 ||
                            (g_bExpandedShipGraphicsEnabled != 0 &&
                             g_aeObjectClass[ship] ==
                                 OBJECT_CLASS_SHIP))
                            viewFrame = g_asLoadedShipViewFrame[ship];
                        tableOffset = exhaustTable[viewFrame];
                        if (tableOffset != -1) {
                            exhaustTable =
                                (short *)((unsigned char *)exhaustTable +
                                          tableOffset);
                            while (*exhaustTable != -1) {
                                object = find_vacant_3d_object();
                                if (object == -1)
                                    return;
                                set_objects_data(object, 0x2b, ship, 0);
                                shapeFrame = *exhaustTable++;
                                scale = *exhaustTable++;
                                if (g_aeObjectClass[ship] >=
                                        OBJECT_CLASS_CAPITAL_SHIP)
                                    scale = (short)(scale +
                                        (unsigned short)RandomInRange(0x10,
                                                                      0x30));
                                else
                                    scale = (short)(scale +
                                        (unsigned short)RandomInRange(0,
                                                                      0x20));
                                if (g_acShipExhaustHeat[ship] == 0)
                                    scale = (short)(scale - 0x20);
                                g_asObjectScale[object] = scale;
                                g_asObjectDistance[object] =
                                    *exhaustTable++;
                                g_asObjectScreenAngle[object] =
                                    flip_angle(ship, *exhaustTable++);
                                g_asObjectFlip[object] = 0;
                                g_asObjectScreenX[object] =
                                    *exhaustTable++;
                                g_asObjectScreenY[object] =
                                    *exhaustTable++;
                                if (g_aeSpecialManeuver[ship] ==
                                        SPECIAL_MANEUVER_AFTERBURNER) {
                                    g_asObjectViewFrame[object] =
                                        (short)(shapeFrame * 3 +
                                                (unsigned short)RandomInRange(
                                                    0, 2));
                                } else {
                                    g_asObjectViewFrame[object] =
                                        (short)((unsigned short)RandomInRange(
                                                    0, 1) +
                                                shapeFrame * 2 + 12);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

/* Function start: 0x416DC3 */
void reposition_fixed_child_objects(void)
{
    short angle;
    short child;
    int up;
    short owner;
    int right;
    long sine;
    long cosA;
#ifdef SDL_PORT
    int fixedCosine;
    int fixedSine;
    float attachmentRight;
    float attachmentUp;
    float cosineFloat;
    float parentScreenX;
    float parentScreenY;
    float sineFloat;
#endif

    for (child = 10; child <= SPACE_LAST_MOVING_OBJECT; child++) {
        if (g_aeObjectClass[child] ==
            OBJECT_CLASS_FIXED_OBJECT) {
            owner = (short)g_acObjectOwner[child];
            switch (g_asObjectType[child]) {
            case 0x3e:
                g_asObjectDistance[child] =
                    (short)((unsigned short)
                                g_asObjectDistance[owner] -
                            100);
                g_asObjectScreenX[child] =
                    g_asObjectScreenX[owner];
                g_asObjectScreenY[child] =
                    g_asObjectScreenY[owner];
                break;
            case 0x3f:
                g_asObjectDistance[child] =
                    (short)((unsigned short)
                                g_asObjectDistance[owner] +
                            100);
                g_asObjectScreenX[child] =
                    g_asObjectScreenX[owner];
                g_asObjectScreenY[child] =
                    g_asObjectScreenY[owner];
                break;
            case 0x0c:
            case 0x2b:
                angle = g_asObjectScreenAngle[owner];
                sine = SinFixed(angle);
                cosA = CosFixed(angle);
                g_asObjectDistance[child] =
                    (short)((unsigned short)
                                g_asObjectDistance[owner] +
                            (unsigned short)
                                g_asObjectDistance[child]);
                right = g_asObjectScreenX[child];
                right *= (unsigned short)
                    g_asObjectScreenScale[owner];
                if ((g_asObjectFlip[owner] & 0x10) != 0)
                    right = -right;
                up = g_asObjectScreenY[child];
                up *= (unsigned short)
                    g_asObjectScreenScale[owner];
                if ((g_asObjectFlip[owner] & 0x20) != 0)
                    up = -up;
                g_asObjectScreenX[child] = (short)(
                    (MultiplyFixed(right, (int)cosA) -
                     MultiplyFixed(up, (int)sine)) >> 8);
                g_asObjectScreenY[child] = (short)(
                    (MultiplyFixed(up, (int)cosA) +
                     MultiplyFixed(right, (int)sine)) >> 8);
                g_asObjectScreenX[child] +=
                    g_asObjectScreenX[owner];
                g_asObjectScreenY[child] +=
                    g_asObjectScreenY[owner];
#ifdef SDL_PORT
                if (g_asObjectType[child] == 0x2b) {
                    /* Match the anchor to the enhanced owner transform. */
                    parentScreenX =
                        (float)g_15_nStoredScreenViewCentreX +
                        (float)(((double)(g_nScreenWidth & ~1) *
                                 0.5 *
                                 g_aObjectViewPosition[owner].x) /
                                g_aObjectViewPosition[owner].z);
                    parentScreenY =
                        (float)g_37_nViewCenterY +
                        (float)(((double)(g_nScreenWidth & ~1) *
                                 0.5 *
                                 g_aObjectViewPosition[owner].y) /
                                g_aObjectViewPosition[owner].z);
                    GetRLETransformTrig((int)angle * 10,
                                        &fixedCosine, &fixedSine);
                    cosineFloat = (float)fixedCosine / 65536.0f;
                    sineFloat = (float)fixedSine / 65536.0f;
                    attachmentRight = (float)right / 256.0f;
                    attachmentUp = (float)up / 256.0f;
                    SdlSetThrusterScreenPosition(
                        child,
                        parentScreenX + attachmentRight * cosineFloat -
                            attachmentUp * sineFloat,
                        parentScreenY + attachmentRight * sineFloat +
                            attachmentUp * cosineFloat);
                }
#endif
                break;
            }
            g_asObjectScreenScale[child] = (short)(
                (unsigned short)g_asObjectScale[child] *
                (unsigned short)g_asObjectScreenScale[owner] >>
                8);
        }
    }
}

/* Function start: 0x4170F9 */
void housekeep_power_plant_and_fuel(short ship)
{
    if (0 < g_anShipSpeed[ship])
        drain_fuel(ship, 5);
}

void ScaleFixedVector(FixedVector *vector, int scale, FixedVector *result);
short HasShipCockpitGunDisplay(short ship);

/* Function start: 0x417124 */
void replenish_shields(short ship)
{
    signed char shield;

    if (ship == 0 && g_acPlayerComponentDamage[1] > 0 &&
        g_nSpaceFrame %
            (g_acPlayerComponentDamage[1] + 1) != 0)
        return;
    for (shield = 0; shield <= 1; shield++) {
        if (g_aasShipMaximumShield[ship][shield] <
            g_aasShipShield[ship][shield])
            g_aasShipShield[ship][shield] =
                g_aasShipMaximumShield[ship][shield];
        if (g_aasShipMaximumShield[ship][shield] >
                g_aasShipShield[ship][shield] &&
            g_nSpaceFrame %
                g_aObjectTypeData[
                    g_acObjectType[ship]].animationDelay == 0) {
            g_aasShipShield[ship][shield]++;
        }
    }
}

/* Function start: 0x41724A */
void replenish_weapon_energy_bank(short ship)
{
    short maximumShield;
    short shieldEnergy;

    if (ship == 0 && g_acPlayerComponentDamage[1] != 0 &&
        (int)(unsigned short)RandomInRange(0, 4) <
            g_acPlayerComponentDamage[1])
        return;
    if (ship == 0 && HasShipCockpitGunDisplay(0) != 0) {
        if (g_nAimCameraMode == 0) {
            g_1_nGunShowEnergyPercent = MinShort(
                (short)(g_1_nGunShowEnergyPercent + 2), 100);
        } else if (g_nTargetCameraZoom < 0x21 &&
                   g_nSpaceFrame % 2 == 0) {
            g_nTargetCameraZoom = MinShort(
                (short)(g_nTargetCameraZoom + 1), 0x21);
        }
    }
    if (g_asShipWeaponEnergy[ship] < 100) {
        shieldEnergy = (short)(g_aasShipShield[ship][1] +
                               g_aasShipShield[ship][0]);
        maximumShield =
            (short)(g_aasShipMaximumShield[ship][1] +
                    g_aasShipMaximumShield[ship][0]);
        if (maximumShield > shieldEnergy) {
            g_asShipWeaponEnergy[ship] =
                MinShort(
                    (short)(g_asShipWeaponEnergy[ship] + 1),
                    100);
        } else {
            g_asShipWeaponEnergy[ship] =
                MinShort(
                    (short)(g_asShipWeaponEnergy[ship] + 2),
                    100);
        }
        if (g_asShipWeaponEnergy[ship] < -5) {
            g_asShipWeaponEnergy[ship] = MinShort(
                (short)(g_asShipWeaponEnergy[ship] + 1), 100);
        }
    }
}

/* Function start: 0x41740F */
void accelerate(short amount)
{
    if (malf(0) != 0) {
        amount = (short)(amount - 2);
        if (g_nSpaceFrame % 3 == 0)
            PlaySfxWaveFileByNumber(3, -1, 0);
    }
#ifdef SDL_PORT
    /* amount goes negative when the throttle is cut, and shifting a negative
     * value left is undefined in C even though it is what the 386 does. */
    celerate(0, (int)amount * 256);
#else
    celerate(0, (int)amount << 8);
#endif
}

/* Function start: 0x417471 */
void your_afterburner(void)
{
    short time;

    if (g_anShipFuel[0] > 0) {
        if (malf(0) != 0) {
            PlaySfxWaveFileByNumber(3, -1, 0);
        } else {
            if (g_aeSpecialManeuver[0] !=
                    SPECIAL_MANEUVER_AFTERBURNER) {
                time = 8;
            } else if (g_asShipAfterburnerTimer[0] == 0) {
                time = 8;
            } else {
                if (g_asShipAfterburnerTimer[0] > 2)
                    return;
                else
                    time = 2;
            }
            fire_afterburner(0, time);
            if (g_nSpaceFrame >
                g_6_nPendingAfterburnerSfxFrame) {
                g_6_nPendingAfterburnerSfxFrame =
                    (int)g_nSpaceFrame + 6;
                PlaySfxWaveFileByNumber(12, -1, 0);
            }
        }
    }
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
int g_6_nPendingAfterburnerSfxFrame; /* 0x005d3820 */
short g_asWingmanDamageCredit[10]; /* 0x005d3830 */
short g_nAdaptiveDifficulty; /* 0x005d3844 */
short g_7_asShipBurstStageCountdown[10]; /* 0x005d3850 */
int g_3_bAfterburnerSfxActive; /* 0x005d3864 */
signed char g_10_abExplosionEntityDoneBytes[
    SPACE_OBJECT_COUNT]; /* 0x005d3870 */
short g_asPlayerHitsCreditValues[10]; /* 0x005d38c0 */
short g_asMissileProximityFuseValues[16]; /* 0x005d38e0 */
short g_1_asObjectCreationFrameTable[SPACE_OBJECT_COUNT]; /* 0x005d3900 */
