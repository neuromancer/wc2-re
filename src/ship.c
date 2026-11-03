/*
 *  Space objects: creation, weapons, movement and damage.
 *
 *  Retail object 0x410020-0x41754F (link order; see docs/ORDER.md).
 */
#include "game.h"

static const short g_aaeExplosionDebris_00492dd8[4][7] = {
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
};

/* Function start: 0x410020 */
short CountShipProjectiles(short ship)
{
    short count;
    short obj;

    count = 0;
    for (obj = 0; obj < SPACE_OBJECT_COUNT; obj++) {
        if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_PROJECTILE &&
            g_acObjectOwner_00495208[obj] == ship)
            count++;
    }
    return count;
}

/* Function start: 0x41008E */
short ShipHasCloakingDevice(short ship)
{
    short weapon;
    unsigned char *weapons;

    weapons = g_aShipWeapons_004956b0[ship];
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
    if (g_aeObjectClass_00495328[ship] == OBJECT_CLASS_SHIP) {
        if (g_aasShipShield_00495518[target][0] >= 1000 ||
            g_aasShipShield_00495518[target][1] >= 1000)
            return 0;
    }
    return 1;
}

/* Function start: 0x410161 */
short IsCapitalShipObject(short obj)
{
    if (g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_CAPITAL_SHIP)
        return 1;
    return 0;
}

/* Function start: 0x410192 */
short HasShipCockpitGunDisplay(short ship)
{
    short weapon;
    short count;

    count = (short)(signed char)g_aShipWeapons_004956b0[ship][0];
    weapon = 0;
    for (; count > weapon; weapon++) {
        if (((ShipWeaponSlot *)&g_aShipWeapons_004956b0[ship][1])[
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

    weapons = g_aShipWeapons_004956b0[ship];
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
    if (g_nCurrentMission_005c5878 >= 0 &&
        g_nCurrentMission_005c5878 < 4) {
        if (g_nCurrentSeries_005c5870 == 0) {
            turnRate = g_aasSeriesMissionTurnRate_004930a0[0][
                g_nCurrentMission_005c5878];
        } else if (g_nCurrentSeries_005c5870 > 0 &&
                   g_nCurrentSeries_005c5870 <= 13) {
            turnRate = g_asDifficultyLevels_004930a8[
                (g_nCurrentSeries_005c5870 - 1) * 4 +
                g_nCurrentMission_005c5878];
        }
    }
#else
    turnRate = g_aasSeriesMissionTurnRate_004930a0[
        g_nCurrentSeries_005c5870][g_nCurrentMission_005c5878];
#endif
    turnRate = (short)((int)turnRate * g_nAdaptiveDifficulty_005d3844 / 5);
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
    if (g_nSpaceFrame_00493134 -
            g_nLastAdaptiveDifficultyChangeFrame_00492d60 < 20)
        return;
    g_nAdaptiveDifficulty_005d3844++;
    if (g_nAdaptiveDifficulty_005d3844 > 10)
        g_nAdaptiveDifficulty_005d3844 = 10;
    g_nLastAdaptiveDifficultyChangeFrame_00492d60 =
        g_nSpaceFrame_00493134;
}

/* Function start: 0x410352 */
void DecreaseAdaptiveDifficulty(void)
{
#ifdef SDL_PORT
    if (g_bSdlBalancedDifficulty != 0)
        return;
#endif
    if (g_nSpaceFrame_00493134 -
            g_nLastAdaptiveDifficultyChangeFrame_00492d60 < 20)
        return;
    g_nAdaptiveDifficulty_005d3844--;
    if (g_nAdaptiveDifficulty_005d3844 < 0)
        g_nAdaptiveDifficulty_005d3844 = 0;
    g_nLastAdaptiveDifficultyChangeFrame_00492d60 =
        g_nSpaceFrame_00493134;
}

/* Function start: 0x4103A6 */
short initialize_object(short obj, short type, short owner,
                        short matchLoadedType)
{
    if (obj != -1) {
        set_objects_data(obj, type, owner, matchLoadedType);
        zero_vector(&g_aShipPosition_00494550[obj]);
        zero_vector(&g_aShipVelocity_00494898[obj]);
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
        if (g_aeObjectClass_00495328[i] == OBJECT_CLASS_DUST) {
            found++;
            break;
        }
    }
    if (!found) {
        for (i = 10; i <= 66; i++) {
            if (g_aeObjectClass_00495328[i] == OBJECT_CLASS_DEBRIS) {
                found++;
                break;
            }
        }
    }
    if (!found) {
        for (i = 10; i <= 66; i++) {
            if (g_aeObjectClass_00495328[i] == OBJECT_CLASS_PROJECTILE &&
                g_acObjectOwner_00495208[i] != 0 &&
                g_asObjectScreenX_00493598[i] == (short)0x8001) {
                found++;
                break;
            }
        }
    }
    if (!found) {
        for (i = 10; i <= 66; i++) {
            if ((g_aeObjectClass_00495328[i] == OBJECT_CLASS_ASTEROID ||
                 g_aeObjectClass_00495328[i] == OBJECT_CLASS_MINE) &&
                g_asObjectScreenX_00493598[i] == (short)0x8001) {
                found++;
                break;
            }
        }
    }
    if (!found) {
        i = -1;
    } else {
        g_asObjectScreenX_00493598[i] = (short)0x8001;
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
        g_asShipSide_004955d0[obj] = SIDE_NEUTRAL;
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
        if (g_aObjectTypeData_00496d30[
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

    slots = &((struct ShipLoadout *)g_aShipWeapons_004956b0)[obj];
    dataType = ((ShipWeaponSlot *)(g_aShipWeapons_004956b0[obj] + 1))[
        weapon].weaponType;
    removedType = ((ShipWeaponSlot *)(g_aShipWeapons_004956b0[obj] + 1))[
        weapon].type;
    objectClass = g_aObjectTypeData_00496d30[dataType].objectClass;
    if (slots->count - 1 > weapon) {
        for (index = weapon; slots->count - 1 > index; index++) {
            slots->slots[index].type = slots->slots[index + 1].type;
            slots->slots[index].mount = slots->slots[index + 1].mount;
            slots->slots[index].disabled = slots->slots[index + 1].disabled;
            slots->slots[index].weaponType =
                slots->slots[index + 1].weaponType;
            if (obj == 0) {
                g_aWeaponDisplayPositions_005d1de0[index] =
                    g_aWeaponDisplayPositions_005d1de0[index + 1];
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
                g_nSelectedReleaseWeaponIndex_004934e0 = -1;
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
    short weaponIndex;
    short weaponCount;

    weaponCount = (short)(signed char)
        g_aObjectTypeData_00496d30[
            g_acObjectType_00493980[obj]].weaponLoadout[0];
    for (weaponIndex = 0; weaponIndex < weaponCount; weaponIndex++) {
        g_aShipWeapons_004956b0[obj][
            weaponIndex * sizeof(ShipWeaponSlot) + 1 +
            offsetof(ShipWeaponSlot, weaponType)] =
            g_aShipWeapons_004956b0[obj][
                weaponIndex * sizeof(ShipWeaponSlot) + 1 +
                offsetof(ShipWeaponSlot, type)];
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

    g_asObjectCreationFrame_005d3900[obj] = g_nSpaceFrame_00493134;
    g_abProjectileCollisionBonus_004960a8[obj] = 0;
    if (matchResourceType != 0) {
        for (weapon = 0; weapon < 5; weapon++) {
            if (g_aObjectTypeData_00496d30[weapon].resourceType == type)
                break;
        }
        typeData = &g_aObjectTypeData_00496d30[weapon];
        g_acObjectType_00493980[obj] = (signed char)weapon;
        g_apObjectExhaustShape_004953b8[obj] = typeData->animation;
        g_apObjectShape_00493868[obj] = typeData->shapeSet;
    } else {
        if (type == OBJECT_DATA_SPACE_DUST) {
            g_asObjectType_00495298[obj] = OBJECT_DATA_SPACE_DUST;
            g_aeObjectClass_00495328[obj] = OBJECT_CLASS_DUST;
            return;
        }
        if (g_aObjectTypeData_00496d30[type].shapeSet == 0) {
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
            g_acObjectType_00493980[obj] = OBJECT_DATA_SPACE_DUST;
        else
            g_acObjectType_00493980[obj] = (signed char)type;
        if (type == 0x3e) {
            g_acObjectType_00493980[obj] =
                (signed char)g_nObjectType62Index_00492d64;
        }
        if (type == 0x3f) {
            g_acObjectType_00493980[obj] =
                (signed char)g_nObjectType63Index_00492d68;
        }
        typeData =
            &g_aObjectTypeData_00496d30[g_acObjectType_00493980[obj]];
        source = typeData;
        if (type == 0xe)
            source = &g_aObjectTypeData_00496d30[10];
        if (type == 0xd)
            source = &g_aObjectTypeData_00496d30[8];
        g_asObjectType_00495298[obj] = type;
        if (type == OBJECT_DATA_CHAFF_POD &&
            g_aObjectTypeData_00496d30[20].shapeSet == 0) {
            g_apObjectShape_00493868[obj] =
                g_aObjectTypeData_00496d30[9].shapeSet;
        }
        if (type == OBJECT_DATA_ROCK_CHUNK) {
            g_apObjectShape_00493868[obj] =
                g_aObjectTypeData_00496d30[
                    OBJECT_DATA_ASTEROID1].shapeSet;
        } else {
            g_apObjectShape_00493868[obj] = source->shapeSet;
        }
    }
    g_asObjectType_00495298[obj] = typeData->field_18;
    g_aeObjectClass_00495328[obj] = typeData->objectClass;
    if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_MISSILE) {
        if (g_asShipSide_004955d0[owner] == SIDE_KILRATHI && owner != 0 &&
            g_pGenericMissileShape_0049c8f0 != 0 &&
            g_pGenericMissileExhaustShape_0049c8f4 != 0 &&
            g_asObjectType_00495298[obj] != OBJECT_DATA_CHAFF_POD) {
            g_apObjectShape_00493868[obj] = g_pGenericMissileShape_0049c8f0;
            g_apObjectExhaustShape_004953b8[obj] =
                g_pGenericMissileExhaustShape_0049c8f4;
        } else {
            g_apObjectExhaustShape_004953b8[obj] = typeData->animation;
        }
    }
    init_ijk(obj);
    g_asObjectCollisionRadius_004950e8[obj] = typeData->collisionRadius;
    g_asObjectScale_00494d90[obj] = typeData->scale;
    if (type == 0xe) {
        g_asObjectScale_00494d90[obj] =
            (unsigned short)(g_asObjectScale_00494d90[obj] * 8);
    }
    if (type == 0xd) {
        g_asObjectScale_00494d90[obj] =
            (unsigned short)(g_asObjectScale_00494d90[obj] * 8);
    }
    g_asObjectDamage_00495178[obj] = 0;
    g_asObjectFlip_004939c8[obj] = g_asObjectDamage_00495178[obj];
    g_asObjectScreenAngle_004936b8[obj] = g_asObjectFlip_004939c8[obj];
    g_acObjectOwner_00495208[obj] = (signed char)owner;
    g_acLastCollisionObject_00495250[obj] = -1;
    if (g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_MISSILE) {
        g_asObjectViewFrame_00493508[obj] = 0;
        g_acShipTarget_00495f20[obj] = -1;
        if (g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_SHIP) {
            g_aasShipShield_00495518[obj][0] = typeData->shieldFore;
            g_aasShipMaximumShield_004954f0[obj][0] =
                g_aasShipShield_00495518[obj][0];
            g_aasShipShield_00495518[obj][1] = typeData->shieldAft;
            g_aasShipMaximumShield_004954f0[obj][1] =
                g_aasShipShield_00495518[obj][1];
            g_aasShipArmor_00495540[obj][0] = typeData->armorFront;
            g_aasShipArmor_00495540[obj][2] = typeData->armorLeft;
            g_aasShipArmor_00495540[obj][3] = typeData->armorRight;
            g_aasShipArmor_00495540[obj][1] = typeData->armorRear;
            g_anShipFuel_00495638[obj] = *(int *)&typeData->lifetime;
            g_acShipIonDriveDamage_004956a0[obj] = 0;
            g_acShipDamage_00495690[obj] = 0;
            recalc_max_velocity(obj);
            DAT_00495d78[obj] = 4;
            memcpy(g_aShipWeapons_004956b0[obj], typeData->weaponLoadout,
                   sizeof(typeData->weaponLoadout));
            InitializeShipWeaponTypeIndices(obj);
            if (obj == 0) {
                g_nSelectedReleaseWeaponIndex_004934e0 = -1;
                g_nSelectedGunType_004934dc = -1;
                weapon = (short)(signed char)g_aShipWeapons_004956b0[obj][0];
                while (weapon-- > 0) {
                    if (((ShipWeaponSlot *)(
                             g_aShipWeapons_004956b0[obj] + 1))[
                            weapon].disabled == 0) {
                        if (g_aObjectTypeData_00496d30[
                                ((ShipWeaponSlot *)(
                                     g_aShipWeapons_004956b0[obj] + 1))[
                                    weapon].weaponType].objectClass ==
                            OBJECT_CLASS_PROJECTILE) {
                            if (g_nSelectedGunType_004934dc != -1 &&
                                ((ShipWeaponSlot *)(
                                     g_aShipWeapons_004956b0[obj] + 1))[
                                    weapon].type !=
                                    g_nSelectedGunType_004934dc) {
                                g_nSelectedGunType_004934dc = 0x80;
                            } else {
                                g_nSelectedGunType_004934dc =
                                    ((ShipWeaponSlot *)(
                                         g_aShipWeapons_004956b0[obj] + 1))[
                                        weapon].type;
                            }
                        } else {
                            g_nSelectedReleaseWeaponIndex_004934e0 = weapon;
                        }
                    }
                }
            }
            if (HasShipCockpitGunDisplay(obj) != 0)
                g_asShipTurretFireEnabled_0049d470[obj] = 1;
            else
                g_asShipTurretFireEnabled_0049d470[obj] = 0;
            g_asShipWeaponEnergy_00495590[obj] = 100;
            g_acShipLastAttacker_004955c0[obj] = -1;
        }
    } else {
        animation = typeData->animation;
        if (animation == 0) {
            frameIndex = 0;
            g_asObjectViewFrame_00493508[obj] =
                (short)(typeData->yawRate + frameIndex);
        } else {
            g_acObjectAnimationDelay_00494d00[obj] = 1;
            g_asObjectAnimationIndex_00494c70[obj] = 0;
            frameIndex = g_asObjectAnimationIndex_00494c70[obj];
            animate_shape(obj);
        }
    }
}

short HasShipCockpitGunDisplay(short ship);
void rotate_object_to_goal(short obj);

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

    typeData = &g_aObjectTypeData_00496d30[g_acObjectType_00493980[obj]];
    if (g_aeSpecialManeuver_00495600[obj] ==
            SPECIAL_MANEUVER_BLOWING_UP) {
        if ((short)alert_flag(obj, 1) != 0) {
            set_special(obj, SPECIAL_MANEUVER_NONE);
        } else {
            /* The original passes a difficulty the callee ignores. */
            if (g_asObjectCounter_00494be0[obj] == -1 &&
                skill_check(obj, 7) != 0)
                g_aeSpecialManeuver_00495600[obj] = SPECIAL_MANEUVER_NONE;
            return;
        }
    }
    totalError = (short)(abs(g_anObjectRollRotation_00495058[obj] -
                             g_anRollGoal_004954d8[obj]) +
                         abs(g_anObjectYawRotation_00494fc8[obj] -
                             g_anYawGoal_004954c0[obj]) +
                         abs(g_anObjectPitchRotation_00494f38[obj] -
                             g_anPitchGoal_004954a8[obj]));
    rate = typeData->yawRate;
    turnScale = MinShort(180, MaxShort(100, GetAdaptiveTurnRate()));
    if (g_asShipSide_004955d0[obj] != g_asShipSide_004955d0[0] &&
        g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP)
        rate = (short)(turnScale * rate / 100);
    match_rotation_goal(&g_anObjectPitchRotation_00494f38[obj],
                        &g_anPitchGoal_004954a8[obj], totalError,
                        typeData->yawRate);
    rate = typeData->pitchRate;
    if (g_asShipSide_004955d0[obj] != g_asShipSide_004955d0[0] &&
        g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP)
        rate = (short)(turnScale * rate / 100);
    match_rotation_goal(&g_anObjectYawRotation_00494fc8[obj],
                        &g_anYawGoal_004954c0[obj], totalError,
                        typeData->pitchRate);
    rate = typeData->rollRate;
    if (g_asShipSide_004955d0[obj] != g_asShipSide_004955d0[0] &&
        g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP)
        rate = (short)(turnScale * rate / 100);
    match_rotation_goal(&g_anObjectRollRotation_00495058[obj],
                        &g_anRollGoal_004954d8[obj], totalError,
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
        g_aeSpecialManeuver_00495600[obj] != SPECIAL_MANEUVER_UNKNOWN_9) {
        if (g_asShipSide_004955d0[obj] != g_asShipSide_004955d0[0] &&
            g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP)
            turnScale = MinShort(180, MaxShort(100, GetAdaptiveTurnRate()));
        else
            turnScale = 100;
        type = g_acObjectType_00493980[obj];
        turnAbility = (short)(g_aObjectTypeData_00496d30[type].pitchRate *
                           turnScale / 100);
        turnAbility = (short)(g_aObjectTypeData_00496d30[type].yawRate *
                           turnScale / 100 + turnAbility);
        turnAbility = (short)(g_aObjectTypeData_00496d30[type].rollRate *
                           turnScale / 100 + turnAbility);
        overshoot = (short)(abs(g_anObjectRollRotation_00495058[obj]) +
                            abs(g_anObjectYawRotation_00494fc8[obj]) +
                            abs(g_anObjectPitchRotation_00494f38[obj]));
        overshoot = (short)(overshoot / turnAbility);
        /* The original passes the overshoot the callee ignores. */
        if (skill_check(obj, overshoot) ==
            0) {
            set_special(obj, SPECIAL_MANEUVER_BLOWING_UP);
            g_asObjectCounter_00494be0[obj] =
                (short)(RandomBelowOrEqual(6) + 5);
        }
    }
}

#pragma intrinsic(abs)
/* Function start: 0x4117AC */
void celerate(short ship, int delta)
{
    int maximumSpeed;

    maximumSpeed = (int)g_asShipMaximumVelocity_00495f70[ship] << 8;
    g_anShipSpeed_00494e20[ship] += delta;
    if (g_anShipSpeed_00494e20[ship] > maximumSpeed)
        g_anShipSpeed_00494e20[ship] = maximumSpeed;
    if (g_anShipSpeed_00494e20[ship] < 0)
        g_anShipSpeed_00494e20[ship] = 0;
}

#pragma function(abs)
/* Function start: 0x41181D */
unsigned int approach_speed(short ship, int targetSpeed)
{
    int delta;
    int acceleration;

    acceleration = GetShipAccelerationRate(ship);
    delta = targetSpeed - g_anShipSpeed_00494e20[ship];

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
    g_anRollGoal_004954d8[ship] = g_anPitchGoal_004954a8[ship] =
        g_anYawGoal_004954c0[ship] = 0;
}

/* Function start: 0x4118F2 */
short real_velocity(short obj)
{
    return FixedToShortSaturating(
        Vector_magnitude(&g_aShipVelocity_00494898[obj]));
}

/* Function start: 0x411922 */
unsigned int fix_velocity(short obj)
{
    ScaleFixedVector(&g_aShipForwardVector_00494208[obj],
                     g_anShipSpeed_00494e20[obj],
                     &g_aShipVelocity_00494898[obj]);
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

    if (g_cViableTargetCount_00496178 > 1) {
        for (outer = 0;
             outer < (short)g_cViableTargetCount_00496178 - 1;
             outer++) {
            for (j = (short)(outer + 1);
                 j < (short)g_cViableTargetCount_00496178;
                 j++) {
                range = g_asViableTargetDistance_00496190[outer];
                if (g_asViableTargetDistance_00496190[j] < range) {
                    temp = g_acViableTarget_00496180[outer];
                    g_asViableTargetDistance_00496190[outer] =
                        g_asViableTargetDistance_00496190[j];
                    g_acViableTarget_00496180[outer] =
                        g_acViableTarget_00496180[j];
                    g_asViableTargetDistance_00496190[j] = range;
                    g_acViableTarget_00496180[j] = (signed char)temp;
                }
            }
        }
    }
}

/* Function start: 0x411A50 */
void send_appropriate_message(short attacker, short victim)
{
    if (g_acObjectOwner_00495208[attacker] != -1 &&
        g_aeObjectClass_00495328[
            (short)g_acObjectOwner_00495208[attacker]] >=
            OBJECT_CLASS_SHIP) {
        if (g_asShipSide_004955d0[
                (short)g_acObjectOwner_00495208[attacker]] ==
                SIDE_IMPERIAL &&
            g_asShipSide_004955d0[
                (short)g_acObjectOwner_00495208[attacker]] !=
                g_asShipSide_004955d0[victim]) {
            if (g_aeObjectClass_00495328[victim] >=
                    OBJECT_CLASS_CAPITAL_SHIP &&
                g_aeSpecialManeuver_00495600[
                    (short)g_acObjectOwner_00495208[attacker]] !=
                    SPECIAL_MANEUVER_UNKNOWN_9)
                send_message(
                    (short)g_acObjectOwner_00495208[attacker], 6);
            if (((short)g_acObjectOwner_00495208[attacker] ==
                     g_nYourWingman_0049346c ||
                 RandomBelowOrEqual(100) < 70) &&
                g_aeSpecialManeuver_00495600[
                    (short)g_acObjectOwner_00495208[attacker]] !=
                    SPECIAL_MANEUVER_UNKNOWN_9 &&
                g_aeObjectClass_00495328[victim] == OBJECT_CLASS_SHIP) {
                if ((short)g_acObjectOwner_00495208[attacker] ==
                    g_nYourWingman_0049346c) {
                    if (g_asPlayerDamageCredit_005d38c0[victim] <=
                            g_asWingmanDamageCredit_005d3830[victim] ||
                        g_aeObjectClass_00495328[attacker] ==
                            OBJECT_CLASS_MISSILE)
                        send_message(
                            (short)g_acObjectOwner_00495208[attacker], 5);
                } else {
                    send_message(
                        (short)g_acObjectOwner_00495208[attacker], 5);
                }
            }
        } else if (g_asShipSide_004955d0[
                       (short)g_acObjectOwner_00495208[attacker]] !=
                       SIDE_IMPERIAL &&
                   g_asShipSide_004955d0[
                       (short)g_acObjectOwner_00495208[attacker]] !=
                       g_asShipSide_004955d0[victim] &&
                   g_aeSpecialManeuver_00495600[
                       (short)g_acObjectOwner_00495208[attacker]] !=
                       SPECIAL_MANEUVER_UNKNOWN_9) {
            send_message((short)g_acObjectOwner_00495208[attacker], 0x1c);
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

    if (g_acObjectOwner_00495208[attacker] == 0)
        destroyed = 0;
    if (g_bApplyingCollisionDamage_00492fb8 != 0 &&
        g_asShipSide_004955d0[victim] == SIDE_KILRATHI &&
        g_acObjectOwner_00495208[attacker] == 0 &&
        RandomBelowOrEqual(9) == 0)
        IncreaseAdaptiveDifficulty();
    if (g_bApplyingCollisionDamage_00492fb8 != 0 && victim == 0 &&
        g_acObjectOwner_00495208[attacker] != -1 &&
        g_asShipSide_004955d0[
            (short)g_acObjectOwner_00495208[attacker]] ==
            SIDE_KILRATHI &&
        RandomBelowOrEqual(5) == 0)
        DecreaseAdaptiveDifficulty();
    if (g_bPlayerDamageEnabled_0049d77c == 0 && victim == 0)
        return 0;
    if ((g_aasShipShield_00495518[victim][0] >= 1000 ||
         g_aasShipShield_00495518[victim][1] >= 1000) &&
        attacker != -1 && g_asObjectType_00495298[attacker] != 0x0d &&
        g_asObjectType_00495298[attacker] != 0x0e &&
        g_asObjectType_00495298[attacker] != 0x13)
        return 0;
    if (attacker != -1 &&
        g_aeObjectClass_00495328[attacker] == OBJECT_CLASS_PROJECTILE &&
        CanShipWeaponDamageTarget(
            (short)g_acObjectOwner_00495208[attacker], victim) == 0)
        return 0;
    if (damage == 0 ||
        g_aeSpecialManeuver_00495600[victim] ==
            SPECIAL_MANEUVER_UNKNOWN_9 ||
        g_aeObjectClass_00495328[victim] < OBJECT_CLASS_MISSILE)
        return 0;

    if (g_aeObjectClass_00495328[victim] < OBJECT_CLASS_SHIP) {
        g_asObjectDamage_00495178[victim] =
            (short)(g_asObjectDamage_00495178[victim] + damage);
        if (g_aObjectTypeData_00496d30[
                g_acObjectType_00493980[victim]].damageCapacity == -1)
            return 0;
        if (g_aObjectTypeData_00496d30[
                g_acObjectType_00493980[victim]].damageCapacity <=
            g_asObjectDamage_00495178[victim])
            return explode(attacker, victim);
    } else {
        if (attacker != -1 &&
            g_acObjectOwner_00495208[attacker] != -1 &&
            g_asShipSide_004955d0[victim] == SIDE_KILRATHI &&
            g_asShipSide_004955d0[
                (short)g_acObjectOwner_00495208[attacker]] ==
                SIDE_KILRATHI &&
            g_aeObjectClass_00495328[
                (short)g_acObjectOwner_00495208[attacker]] >=
                OBJECT_CLASS_CAPITAL_SHIP)
            return 0;
        if (victim == 0) {
            TriggerPlayerHitPaletteFlash();
#ifdef SDL_PORT
            SdlQueueJoystickDamageRumble(damage);
#endif
        }
        if (g_acObjectOwner_00495208[attacker] != -1 &&
            g_aeObjectClass_00495328[
                (short)g_acObjectOwner_00495208[attacker]] ==
                OBJECT_CLASS_SHIP &&
            g_asShipSide_004955d0[
                (short)g_acObjectOwner_00495208[attacker]] ==
                g_asShipSide_004955d0[victim])
            g_asShipFriendlyFireCooldown_00496090[
                (short)g_acObjectOwner_00495208[attacker]] = 40;
        if ((short)g_acObjectOwner_00495208[attacker] ==
            g_nYourWingman_0049346c)
            g_asWingmanDamageCredit_005d3830[victim]++;
        if (g_acObjectOwner_00495208[attacker] == 0)
            g_asPlayerDamageCredit_005d38c0[victim] =
                (short)(g_asPlayerDamageCredit_005d38c0[victim] + 2);
        if (attacker == g_nYourWingman_0049346c && victim == 0)
            send_message(g_nYourWingman_0049346c, 0x0c);
        if (attacker == 0 && victim == g_nYourWingman_0049346c)
            send_message(g_nYourWingman_0049346c, 0x0c);
        if (g_asObjectCreationFrame_005d3900[attacker] !=
                g_nLastFriendlyFireObjectFrame_00492d6c &&
            attacker != -1 &&
            g_asShipSide_004955d0[victim] == SIDE_IMPERIAL &&
            g_acObjectOwner_00495208[attacker] == 0 &&
            (g_aeObjectClass_00495328[attacker] != OBJECT_CLASS_MISSILE ||
             g_acShipTarget_00495f20[attacker] == victim)) {
            if (kilrathi_near(0, 30000) != 0) {
                send_message(victim, 0x0c);
                g_nLastFriendlyFireObjectFrame_00492d6c =
                    g_asObjectCreationFrame_005d3900[attacker];
            } else if (g_asShipSide_004955d0[0] != SIDE_KILRATHI) {
                if (g_bFriendlyFireWarningIssued_00492d5c == 0) {
                    g_bFriendlyFireWarningIssued_00492d5c++;
                    send_message(victim, 0x0d);
                    g_nLastFriendlyFireObjectFrame_00492d6c =
                        g_asObjectCreationFrame_005d3900[attacker];
                } else {
                    send_message(victim, 0x0e);
                    g_nLastFriendlyFireObjectFrame_00492d6c =
                        g_asObjectCreationFrame_005d3900[attacker];
                    g_asShipSide_004955d0[0] = SIDE_KILRATHI;
                    g_nYourWingman_0049346c = -1;
                    for (ally = 1; ally < 10; ally++) {
                        if (g_asShipSide_004955d0[ally] == SIDE_IMPERIAL) {
                            if (g_aeObjectClass_00495328[ally] ==
                                OBJECT_CLASS_SHIP) {
                                g_asShipMissionType_00495de8[ally] =
                                    MISSION_TYPE_STRIKE;
                                g_asShipMissionParameter_00495e00[ally] =
                                    g_asShipMissionIndex_00495d00[0];
                                g_aeShipObjective_00495f08[ally] =
                                    OBJECTIVE_DESTROY_SHIP;
                                g_acShipTarget_00495f20[ally] = 0;
                            } else if (g_aeObjectClass_00495328[ally] >=
                                       OBJECT_CLASS_CAPITAL_SHIP) {
                                g_asShipMissionType_00495de8[ally] =
                                    MISSION_TYPE_NONE;
                                g_asShipTactic_00495f30[ally] =
                                    TACTIC_SELF_DEFENSE;
                            }
                        }
                    }
                }
            }
        }
        quadrant = (short)(dot_product(
            impactDirection, &g_aShipForwardVector_00494208[victim]) > 0);
        if (g_anShipCloakState_00496020[victim] != 1 &&
            g_asObjectType_00495298[attacker] != 0x13 &&
            g_aasShipShield_00495518[victim][0] < 1000 &&
            g_aasShipShield_00495518[victim][1] < 1000) {
            protection = &g_aasShipShield_00495518[victim][quadrant];
            damage = (short)(damage - *protection);
            if (damage <= 0) {
                *protection = (short)-damage;
                if (attacker != -1 &&
                    g_aeObjectClass_00495328[attacker] ==
                        OBJECT_CLASS_PROJECTILE) {
                    if (victim == 0)
                        PlaySfxWaveFileByNumber(10, 0, 0);
                    else
                        PlaySfxWaveFileByNumber(10, victim, 0);
                }
                return 0;
            }
            *protection = 0;
        }
        if (attacker != -1 &&
            g_aeObjectClass_00495328[attacker] ==
                OBJECT_CLASS_PROJECTILE) {
            if (victim == 0)
                PlaySfxWaveFileByNumber(9, 0, 0);
            else
                PlaySfxWaveFileByNumber(9, victim, 0);
        }
        sideProjection = dot_product(
            impactDirection, &g_aShipRightVector_00493b78[victim]);
        if (sideProjection > 0xb5)
            quadrant = 3;
        else if (sideProjection < -0xb5)
            quadrant = 2;
        protection = &g_aasShipArmor_00495540[victim][quadrant];
        damage = (short)(damage - *protection);
        if (damage > 0) {
            *protection = 0;
            if (g_asObjectScreenX_00493598[victim] != (short)0x8001 &&
                g_aeObjectClass_00495328[victim] <
                    OBJECT_CLASS_CAPITAL_SHIP &&
                RandomBelowOrEqual(1) == 0)
                Create_ship_hit_debris(attacker, 1);
            if (RandomBelowOrEqual(99) == 0) {
                if (g_acObjectOwner_00495208[attacker] == 0) {
                    if (g_asShipSide_004955d0[victim] == SIDE_KILRATHI)
                        send_message(victim, 0x1d);
                    destroyed = explode(attacker, victim);
                }
            } else {
                destroyed = internal_damage(
                    attacker,
                    (short)g_acObjectOwner_00495208[attacker],
                    victim, damage, quadrant);
            }
            if (destroyed == 1) {
                send_appropriate_message(attacker, victim);
                if (g_asShipSide_004955d0[victim] == SIDE_KILRATHI &&
                    g_acObjectOwner_00495208[attacker] == 0)
                    IncreaseAdaptiveDifficulty();
                if (g_nYourWingman_0049346c == victim)
                    DecreaseAdaptiveDifficulty();
            }
            return destroyed;
        }
        *protection = (short)-damage;
        return 0;
    }
    return 0;
}

/* Function start: 0x41262D */
short pilot_hit(short obj)
{
    if (DAT_00495d78[obj] > 0) {
        DAT_00495d78[obj]--;
        if (DAT_00495d78[obj] == 0) {
            if (obj == 0) {
                if (g_bPlayerDamageEnabled_0049d77c != 0 &&
                    g_bDeathSequenceActive_0049da50 == 0)
                    g_nArcadeState_0049d75c = 4;
                return -1;
            }
        } else if (skill_check(obj, 9) == 0 && obj != 0) {
            g_asObjectCounter_00494be0[obj] =
                (short)(RandomBelowOrEqual(20) + 30);
            set_special(obj, SPECIAL_MANEUVER_BLOWING_UP);
        }
    }
    return DAT_00495d78[obj];
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
        g_asObjectScale_00494d90[debris] <<= 1;
        g_aShipVelocity_00494898[debris] =
            g_aShipVelocity_00494898[obj];
        g_asObjectCounter_00494be0[debris] = 6;
        MakeRandomVectorFixed(
            (short)(g_asObjectCollisionRadius_004950e8[obj] >> 2),
            (short)(g_asObjectCollisionRadius_004950e8[obj] >> 1),
            &offset);
        AddFixedVectors(&g_aShipPosition_00494550[obj], &offset,
                        &g_aShipPosition_00494550[debris]);
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
        if (g_aeObjectClass_00495328[other] >= OBJECT_CLASS_SHIP &&
            g_aeSpecialManeuver_00495600[other] !=
                SPECIAL_MANEUVER_UNKNOWN_9 &&
            g_asShipSide_004955d0[obj] != g_asShipSide_004955d0[other] &&
            RandomBelow(100) < 50)
            g_acShipTarget_00495f20[other] = obj;
    }
}

/* Function start: 0x4128A7 */
short internal_damage(short attacker, short owner, short victim,
                      short damage, short quadrant)
{
    short system;
    signed char weapon;
    short type;
    short ship;
    short damageCapacity;

    if (victim == 0)
        return your_internal_damage(attacker, owner, damage, quadrant);
    type = (short)g_acObjectType_00493980[victim];
    damageCapacity = g_aObjectTypeData_00496d30[type].damageCapacity;
    if (g_aeObjectClass_00495328[victim] >= OBJECT_CLASS_CAPITAL_SHIP) {
        if (g_asShipSide_004955d0[victim] == SIDE_KILRATHI) {
            if (owner == 0)
                IncreaseAdaptiveDifficulty();
            if (g_asObjectType_00495298[attacker] == 0x0d ||
                g_asObjectType_00495298[attacker] == 0x0e) {
                g_asObjectDamage_00495178[victim] = (short)(
                    g_asObjectDamage_00495178[victim] + damage);
            } else if (g_asObjectType_00495298[attacker] == 0x13 &&
                       damage > 100) {
                g_asObjectDamage_00495178[victim] = (short)(
                    g_asCapitalHitDamageStep_005d17a0[victim] +
                    g_asObjectDamage_00495178[victim]);
            } else {
                g_asObjectDamage_00495178[victim] = (short)(
                    g_asObjectDamage_00495178[victim] +
                    MaxShort(1, (short)(damage >> 3)));
            }
            ship = attacker;
            if (owner != -1)
                ship = owner;
            if (ship != -1 && ship < 10 &&
                any_enemy(ship, 10000) == 0)
                call_enemy(ship);
        } else {
            if (g_asObjectType_00495298[attacker] == 0x0d ||
                g_asObjectType_00495298[attacker] == 0x0e) {
                g_asObjectDamage_00495178[victim] = (short)(
                    g_asObjectDamage_00495178[victim] + damage);
            } else {
                g_asObjectDamage_00495178[victim] = (short)(
                    g_asObjectDamage_00495178[victim] +
                    MaxShort(1, (short)(damage / 10)));
            }
            if (RandomBelowOrEqual(1000) < 35 && attacker != 0 &&
                owner != 0)
                send_message(victim, 4);
        }
        if (g_asObjectDamage_00495178[victim] >= damageCapacity)
            return explode(attacker, victim);
        onboard_explosion(victim);
        return 0;
    }

    if (g_asPilotLevel_00495d60[victim] == 5) {
        damage = MinShort(RandomInRange(3, 4),
                          MaxShort(1, (short)(damage / 40)));
    } else {
        damage = MaxShort(1, (short)(damage / 6));
    }
    g_asObjectDamage_00495178[victim] =
        (short)(g_asObjectDamage_00495178[victim] + damage);

    while (damage > 0) {
        if (damage == 1 && g_acShipPortrait_00495d88[victim] != -1)
            system = 4;
        else
            system = RandomBelowOrEqual(9);
        switch (system) {
        case 8:
            if (quadrant == 0 &&
                g_acShipCommunicator_00495670[victim] != -1) {
                damage--;
                g_acShipCommunicator_00495670[victim] = -1;
            }
            break;
        case 0:
            if (pilot_hit(victim) != 0)
                damage--;
            break;
        case 6:
            if (quadrant == 0 &&
                g_acShipDestroyedWeaponCount_00495680[victim] < 5) {
                damage--;
                g_acShipDestroyedWeaponCount_00495680[victim]++;
            }
            break;
        case 5:
            if (quadrant == 0 &&
                (signed char)g_aShipWeapons_004956b0[victim][0] > 0) {
                weapon = (signed char)RandomBelowOrEqual(
                    (short)((signed char)
                        g_aShipWeapons_004956b0[victim][0] - 1));
                remove_weapon(victim, (short)weapon);
                damage--;
            }
            break;
        case 2:
            if (quadrant == 1)
                return explode(attacker, victim);
            break;
        case 3:
            g_aasShipMaximumShield_004954f0[victim][0] = 0;
            g_aasShipShield_00495518[victim][0] = 0;
            g_aasShipMaximumShield_004954f0[victim][1] = 0;
            g_aasShipShield_00495518[victim][1] = 0;
            break;
        case 4:
            damage--;
            g_acShipDamage_00495690[victim]++;
            if (g_asShipSide_004955d0[victim] == SIDE_KILRATHI &&
                owner == 0)
                IncreaseAdaptiveDifficulty();
            if ((short)g_acShipDamage_00495690[victim] > damageCapacity)
                return explode(attacker, victim);
            break;
        case 7:
            if (quadrant == 1) {
                damage--;
                drain_fuel(victim,
                           (short)(*(int *)&g_aObjectTypeData_00496d30[
                                       type].lifetime / 4));
                if (RandomBelowOrEqual(1) != 0 ||
                    g_anShipFuel_00495638[victim] < 0)
                    return explode(attacker, victim);
            }
            break;
        case 1:
            if (quadrant == 1) {
                damage--;
                damage_ion_drive(victim, 1, 3);
            }
            break;
        }
    }
    return 0;
}

/* Function start: 0x412F03 */
void revise_shields(short obj)
{
    g_aasShipMaximumShield_004954f0[obj][0] =
        MaxShort(0, (short)(g_aasShipMaximumShield_004954f0[obj][0] -
            (g_aObjectTypeData_00496d30[
                g_acObjectType_00493980[obj]].shieldFore >> 2)));
    g_aasShipMaximumShield_004954f0[obj][1] =
        MaxShort(0, (short)(g_aasShipMaximumShield_004954f0[obj][0] -
            (g_aObjectTypeData_00496d30[
                g_acObjectType_00493980[obj]].shieldAft >> 2)));
}

/* Function start: 0x412F90 */
void check_computer_damage(void)
{
    damage_your_component(3, 1, 3);
}

void set_objects_data(short obj, short type, short owner, short matchObjectClass);
void damage_ion_drive(short ship, short amount, short maximum);
void flag_objective(short objective, unsigned char flags);

/* Function start: 0x412FA9 */
short IsOnlyShipGun(short ship, short weapon)
{
    short otherGuns;
    short candidate;

    otherGuns = 0;
    if (g_aObjectTypeData_00496d30[
            (signed char)((ShipWeaponSlot *)
                (g_aShipWeapons_004956b0[ship] + 1))[weapon].weaponType].
            objectClass == OBJECT_CLASS_PROJECTILE) {
        candidate = 0;
        for (; candidate <
             (signed char)g_aShipWeapons_004956b0[ship][0];
             candidate++) {
            if (weapon != candidate &&
                g_aObjectTypeData_00496d30[
                    (signed char)((ShipWeaponSlot *)
                        (g_aShipWeapons_004956b0[ship] + 1))[weapon].
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

    if (g_aeObjectClass_00495328[attacker] ==
        OBJECT_CLASS_PROJECTILE) {
        if (quadrant != 1)
            group = 0;
        else
            group = 2;
        damage = MaxShort(1, (short)(damage >> 4));
    } else if (g_aeObjectClass_00495328[attacker] ==
                   OBJECT_CLASS_ASTEROID ||
               g_aeObjectClass_00495328[attacker] >=
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
    craftType = (short)g_acObjectType_00493980[0];
    roll = (signed char)RandomBelowOrEqual(10);
    g_asObjectDamage_00495178[0] =
        (short)(g_asObjectDamage_00495178[0] + damage);
    if (damage > 1) {
        DecreaseAdaptiveDifficulty();
        place_damage_on_cockpit(RandomBelowOrEqual(3));
    }

    while (damage > 0) {
        damage--;
        system = (signed char)g_aasPlayerDamageSystemTable_00492d70[
            group][RandomBelowOrEqual(9)];
        switch (system) {
        case 8:
            if (quadrant != 0) {
                damage++;
                break;
            }
            if (roll < 7) {
                damage_your_component(4, 2, 3);
                if (g_acPlayerComponentDamage_00493470[4] >= 4)
                    g_acShipDestroyedWeaponCount_00495680[0] = -1;
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
            if (g_acShipCommunicator_00495670[0] < 5) {
                g_acShipCommunicator_00495670[0]++;
                check_computer_damage();
            }
            break;
        case 5:
            if (quadrant != 0) {
                damage++;
                break;
            }
            if ((signed char)g_aShipWeapons_004956b0[0][0] > 1) {
                weapon = (signed char)RandomBelowOrEqual(
                    (short)((signed char)g_aShipWeapons_004956b0[0][0] - 1));
                if (IsOnlyShipGun(0, (short)weapon) == 0) {
                    remove_weapon(0, (short)weapon);
                    ShowComponentHitHudMessage(
                        g_szWeaponDestroyed_00492e20,
                        g_abGamePaletteReservedColours_0049cb54[8], 8);
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
            g_acShipDamage_00495690[0]++;
            if (g_acShipDamage_00495690[0] == 1) {
                /* A stray semicolon in the original: the retail test of
                 * pilot_hit() branches to the decrement either way. */
                if (pilot_hit(0) != 0)
                    ;
                damage--;
            } else if ((short)g_acShipDamage_00495690[0] >
                       g_aObjectTypeData_00496d30[craftType].
                           damageCapacity) {
                return explode(attacker, 0);
            }
            break;
        case 7:
            drain_fuel(0,
                       (short)(*(int *)&g_aObjectTypeData_00496d30[
                                   craftType].lifetime / 4));
            if (RandomBelowOrEqual(1) != 0 ||
                g_anShipFuel_00495638[0] < 0)
                return explode(attacker, 0);
            ShowComponentHitHudMessage(
                g_szFuelTanksHit_00492e34,
                g_abGamePaletteReservedColours_0049cb54[8], 8);
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
        (short)g_acPlayerComponentDamage_00493470[component]) {
        g_acPlayerComponentDamage_00493470[component]--;
        sprintf(message, g_szComponentFixedFormat_00492e44,
                g_apszComponentNames_00490090[component]);
        ShowComponentHitHudMessage(
            message, g_abGamePaletteReservedColours_0049cb54[8], 8);
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
            if (g_acPlayerComponentDamage_00493470[2] < 4)
                ReportComponentRepaired(2, 1);
            break;
        case 1:
            if (g_acPlayerComponentDamage_00493470[0] < 4) {
                if (ReportComponentRepaired(0, 2) != 0)
                    damage_ion_drive(0, -1, 3);
            }
            break;
        case 2:
            if (g_aObjectTypeData_00496d30[
                    g_acObjectType_00493980[0]].damageCapacity - 3 <
                (short)g_acShipDamage_00495690[0])
                g_acShipDamage_00495690[0]--;
            break;
        }
    }
}

void FillFixedVectorWithRandomComponents(short limit, FixedVector *vector);
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
            g_aeShipHitDebrisTypes_00492e10[
                RandomBelowOrEqual(2)], -1, 0);
        g_asObjectCounter_00494be0[debris] = 40;
        FillFixedVectorWithRandomComponents(10, &offset);
        AddFixedVectors(&g_aShipPosition_00494550[obj], &offset,
                        &g_aShipPosition_00494550[debris]);
        FillFixedVectorWithRandomComponents(
            6, &g_aShipVelocity_00494898[debris]);
        RecordCannedSceneObjectEvent(debris, 0);
    }
}

/* Function start: 0x4137C2 */
void check_next_wave(void)
{
    short i;
    MissionObjective *objectiveData;

    for (i = 0; i < 10; i++) {
        if (g_aeObjectClass_00495328[i] >= OBJECT_CLASS_SHIP &&
            g_asShipSide_004955d0[i] == SIDE_KILRATHI)
            return;
    }
    if (g_nEscapedEnemyCount_004962e8 != 0)
        return;
    for (i = 0; i < (short)g_cMissionObjectiveCount_00493294; i++) {
        objectiveData = &g_aMissionObjectives_004932a8[i];
        if (objectiveData->type == 5 &&
            objectiveData->index == g_nCurrentNavPoint_004931bc) {
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
        if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP &&
            g_asShipSide_004955d0[obj] == SIDE_KILRATHI)
            return;
    }
    check_next_wave();
    if (g_ucPendingEjectionTransition_0049b8ac != 0xff &&
        g_bEjectionWaitForEnemyWave_0049b8b0 != 0) {
        if (g_bEjectionTriggerImmediately_0049b8bc != 0) {
            ejection_sequence(
                g_ucPendingEjectionTransition_0049b8ac, 1);
            g_ucPendingEjectionTransition_0049b8ac = 0xff;
            g_bEjectionTriggerImmediately_0049b8bc = 0;
        } else {
            g_nEjectionSequenceState_0049b8c0 = 0;
            g_nPendingEjectionSequenceCount_0049b8b8++;
        }
    }
    if (set_up_next_wave() == 0) {
        for (obj = 1; obj < 10; obj++) {
            if (g_asShipSide_004955d0[obj] == SIDE_KILRATHI &&
                g_aeObjectClass_00495328[obj] != OBJECT_CLASS_NULL)
                return;
        }
        if (g_ucPendingEjectionTransition_0049b8ac != 0xff) {
            g_nEjectionSequenceState_0049b8c0 = 0;
            g_nPendingEjectionSequenceCount_0049b8b8++;
        }
        if (g_bMissionDeathSequencePending_0049b720 != 0 &&
            g_bMissionEjectionPodSpawned_0049b724 != 0) {
            ejection_sequence(
                (short)(g_nPlayerShipType_00493464 + 300), 1);
            g_bMissionDeathSequencePending_0049b720 = 0;
            g_bMissionEjectionPodSpawned_0049b724 = 0;
        }
    }
}

/* Function start: 0x413A3B */
void Create_explosion_debris(short obj)
{
    FixedVector vector;
    short set;
    short i;
    short fragment;
    short index;

    for (i = 0; i < 5; i++) {
        if (g_aObjectTypeData_00496d30[
                g_acObjectType_00493980[obj]].resourceType ==
            g_aObjectResourceSlots_00493398[i].resourceType)
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
                         g_aaeExplosionDebris_00492dd8[set][index], -1, 0);
        if (g_asObjectType_00495298[fragment] == OBJECT_DATA_SHIP_WING)
            g_apObjectShape_00493868[fragment] =
                g_aObjectResourceSlots_00493398[i].field_12;
        g_asObjectCounter_00494be0[fragment] = 40;
        FillFixedVectorWithRandomComponents(50, &vector);
        AddFixedVectors(&g_aShipPosition_00494550[obj], &vector,
                        &g_aShipPosition_00494550[fragment]);
        FillFixedVectorWithRandomComponents(
            25, &g_aShipVelocity_00494898[fragment]);
        divide_vector(&g_aShipVelocity_00494898[obj], 0x200, &vector);
        AddFixedVectors(&vector,
                        &g_aShipVelocity_00494898[fragment],
                        &g_aShipVelocity_00494898[fragment]);
        RecordCannedSceneObjectEvent(fragment, 0);
    }
    for (index = 0; index < 8; index++) {
        fragment = find_vacant_3d_object();
        if (fragment == -1)
            break;
        FillFixedVectorWithRandomComponents(50, &vector);
        AddFixedVectors(&g_aShipPosition_00494550[obj], &vector,
                        &g_aShipPosition_00494550[fragment]);
        FillFixedVectorWithRandomComponents(
            25, &g_aShipVelocity_00494898[fragment]);
        divide_vector(&g_aShipVelocity_00494898[obj], 0x200, &vector);
        AddFixedVectors(&vector,
                        &g_aShipVelocity_00494898[fragment],
                        &g_aShipVelocity_00494898[fragment]);
        g_asObjectScreenAngle_004936b8[fragment] =
            (short)(RandomBelowOrEqual(3) + 0x10);
        g_aeObjectClass_00495328[fragment] = OBJECT_CLASS_DUST;
        g_asObjectType_00495298[fragment] = OBJECT_DATA_DEBRIS_DUST;
        g_acObjectType_00493980[fragment] =
            (signed char)g_asObjectType_00495298[fragment];
        g_asObjectCounter_00494be0[fragment] = 40;
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
    g_nMissionScore_00493462 += score;
}

/* Function start: 0x413E4C */
void score_for_kill(short pilot, short victim)
{
    int event;

    if (g_asShipSide_004955d0[victim] == SIDE_KILRATHI) {
        /* WC2 renumbered the craft table; the Kilrathi hulls that carry a
         * kill score sit at 48-58 and the retail image names none of them. */
        if (g_aeObjectClass_00495328[victim] == OBJECT_CLASS_BASE ||
            g_asObjectType_00495298[victim] == 58) {
            event = 7;
        } else {
            switch (g_asObjectType_00495298[victim]) {
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

short find_ship_index(short missionShip);

/* Function start: 0x413F58 */
void analyze_kill(short attacker, short victim)
{
    short hostile;
    short initialShip;

    hostile = g_asShipSide_004955d0[attacker] !=
            g_asShipSide_004955d0[victim];
    new_space_music_changes(attacker, victim);
    if (hostile != 0) {
        score_for_kill(attacker, victim);
        if (attacker == 0) {
            g_cPlayerKillCount_005d2fa8++;
        } else {
            for (initialShip = 0; initialShip < 8; initialShip++) {
                if (g_stMissionHeader_005d3e70
                        .initialMissionShips[initialShip] ==
                    g_asShipMissionIndex_00495d00[attacker])
                    g_acInitialShipKillCount_005d2fc0[initialShip]++;
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
            g_aeObjectClass_00495328[target] >= OBJECT_CLASS_MISSILE) {
            ComputeVectorDelta(&g_aShipPosition_00494550[obj],
                               &g_aShipPosition_00494550[target], &delta);
            range = MaxShort(0,
                (short)(FixedToShortSaturating(Vector_magnitude(&delta)) -
                        g_asObjectCollisionRadius_004950e8[target]));
            if (range > 1000 ||
                (g_aeObjectClass_00495328[target] == OBJECT_CLASS_MISSILE &&
                 ((g_asMissileProximityFuse_005d38e0[target] + 1) * 100 <
                      range ||
                  g_asShipTactic_00495f30[target] != 4))) {
                damage = 0;
            } else {
                if (range > 750)
                    range = 40;
                else if (range > 500)
                    range = 30;
                else
                    range = find_ratio(0, 500, range, 8, 25);
                damage = (short)((blastDamage / range) / range);
                if (obj != -1 &&
                    g_aeObjectClass_00495328[target] >=
                        OBJECT_CLASS_CAPITAL_SHIP &&
                    g_aeObjectClass_00495328[obj] == OBJECT_CLASS_MISSILE &&
                    g_asObjectType_00495298[obj] != OBJECT_DATA_TORPEDO)
                    damage = 0;
                if (target == 0)
                    damage = MaxShort(1, (short)(damage * 3 >> 2));
            }
            if (damage > 1) {
                NormalizeFixedVector(&delta);
                ScaleFixedVector(&delta, (int)damage << 8, &force);
                apply_force_to_objects_center(&force, target);
                if (g_asObjectType_00495298[obj] != OBJECT_DATA_TORPEDO) {
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

    savedScale = (unsigned short)g_asObjectScale_00494d90[obj];
    explosion = find_vacant_3d_object();
    if (explosion == -1) {
        if (g_aeObjectClass_00495328[obj] >=
            OBJECT_CLASS_CAPITAL_SHIP)
            FreePacketAndClear(&g_apObjectShape_00493868[obj], 0);
        g_asLoadedShipViewFrame_00495d18[obj] = -1;
        explosion = obj;
    } else {
        copy_frame(obj, explosion);
        g_aShipPosition_00494550[explosion] =
            g_aShipPosition_00494550[obj];
        g_aShipVelocity_00494898[explosion] =
            g_aShipVelocity_00494898[obj];
        g_acObjectOwner_00495208[explosion] = (signed char)obj;
    }
    if (g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_CAPITAL_SHIP) {
        PlaySfxWaveFileByNumber(0x31, obj, 0);
        explosionType = OBJECT_DATA_EXPLOSION_MEDIUM;
    } else {
        explosionType = OBJECT_DATA_EXPLOSION_LARGE;
    }
    set_objects_data(explosion, explosionType,
                     (short)g_acObjectOwner_00495208[explosion], 0);
    g_asObjectScale_00494d90[explosion] = (short)(
        (unsigned short)g_asObjectScale_00494d90[explosion] *
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
    if (g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_SHIP) {
        shipIndex = -1;
        if (g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_CAPITAL_SHIP)
            shipIndex = find_ship_index(obj);
        if (obj < 10 &&
            (g_asPilotLevel_00495d60[obj] == 5 ||
             (shipIndex != -1 &&
              g_asShipMissionParameter_00495e00[obj] == shipIndex) ||
             g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_CAPITAL_SHIP ||
             RandomBelow(100) < 40)) {
            if (g_asShipSide_004955d0[obj] == 1)
                send_message(obj, 30);
            else
                send_message(obj, 8);
        }
        for (slot = 0; slot < 8; slot++) {
            if (g_stMissionHeader_005d3e70.initialMissionShips[slot] ==
                g_asShipMissionIndex_00495d00[obj]) {
                g_acMissionShipStatusFlags_005d2fb0[slot] |= 1;
                if (g_nYourWingman_0049346c == obj)
                    g_nYourWingman_0049346c = -1;
            }
        }
        set_special(obj, SPECIAL_MANEUVER_UNKNOWN_9);
        if (g_aObjectTypeData_00496d30[
                OBJECT_DATA_EXPLOSION_LARGE].shapeSet != 0)
            g_asObjectCounter_00494be0[obj] = 8;
        else
            g_asObjectCounter_00494be0[obj] = 4;
        if (g_aObjectTypeData_00496d30[
                OBJECT_DATA_EXPLOSION_LARGE].shapeSet != 0)
            g_asShipExplosionStageTimer_005d3850[obj] = 8;
        else
            g_asShipExplosionStageTimer_005d3850[obj] = 4;
        g_aMissionShips_00492290[
            g_asShipMissionIndex_00495d00[obj]].state = 3;
        if (g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_CAPITAL_SHIP) {
            for (slot = 0; slot < 4; slot++)
                onboard_explosion(obj);
            g_asObjectCounter_00494be0[obj] = (short)(
                (g_aObjectTypeData_00496d30[
                     g_acObjectType_00493980[obj]].damageCapacity >> 2) +
                (g_aObjectTypeData_00496d30[
                     OBJECT_DATA_EXPLOSION_LARGE].shapeSet != 0 ? 8 : 4));
            g_asShipExplosionStageTimer_005d3850[obj] = (short)(
                (g_aObjectTypeData_00496d30[
                     OBJECT_DATA_EXPLOSION_LARGE].shapeSet != 0 ?
                     8 : 4) * 2);
            if (g_asShipExplosionStageTimer_005d3850[obj] <
                g_nHudMessageTime_005d1c32) {
                g_asShipExplosionStageTimer_005d3850[obj] =
                    g_nHudMessageTime_005d1c32;
            }
        } else {
            debris = ShipExplosion(obj);
            explosion_shock_wave(obj,
                g_aObjectTypeData_00496d30[
                    g_acObjectType_00493980[obj]].explosionDamage);
        }
    } else {
        if (g_aeObjectClass_00495328[debris] == OBJECT_CLASS_ASTEROID)
            g_asObjectScale_00494d90[debris] = 0x380;
        explosion_shock_wave(obj,
            g_aObjectTypeData_00496d30[
                g_acObjectType_00493980[obj]].explosionDamage);
        if (g_asObjectType_00495298[obj] == OBJECT_DATA_TORPEDO) {
            debris = obj;
            set_objects_data(debris, OBJECT_DATA_EXPLOSION_LARGE,
                             g_acObjectOwner_00495208[obj], 0);
        } else {
            debris = obj;
            set_objects_data(debris, OBJECT_DATA_EXPLOSION_SMALL,
                             g_acObjectOwner_00495208[obj], 0);
        }
        RecordCannedSceneObjectEvent(debris, 0);
    }
    if (g_asObjectScreenX_00493598[obj] != -32767)
        PlaySfxWaveFileByNumber(4, obj, 0);
    return debris;
}

/* Function start: 0x414835 */
short explode(short attacker, short victim)
{
    short result;
    short object;

    if (victim < 10)
        g_anShipCloakState_00496020[victim] = 2;
    if (g_bExplosionTraversalIdle_00492e18 != 0) {
        for (object = 0; object < SPACE_OBJECT_COUNT; object++)
            g_abExplosionObjectVisited_005d3870[object] = 0;
    }
    g_bExplosionTraversalIdle_00492e18 = 0;
    if (g_abExplosionObjectVisited_005d3870[victim] != 0)
        return 1;
    else
        g_abExplosionObjectVisited_005d3870[victim] = 1;
    result = ResolveObjectDestruction(attacker, victim);
    g_bExplosionTraversalIdle_00492e18 = 1;
    return result;
}

/* Function start: 0x4148F5 */
short ResolveObjectDestruction(short attacker, short victim)
{
    short objective;

    if (victim < 10 && g_asPilotLevel_00495d60[victim] == 5) {
        if (g_abPilotEjectionAttempted_00496110[victim] == 0) {
            g_abPilotEjectionAttempted_00496110[victim]++;
            g_acShipStress_00496100[victim] = 120;
            if (g_asShipSide_004955d0[victim] == SIDE_IMPERIAL)
                reset_maneuver(victim, MANEUVER_OUTA_HERE);
            g_acShipDamage_00495690[victim] =
                (signed char)(g_acShipDamage_00495690[victim] / 2);
            if (g_asShipSide_004955d0[victim] == SIDE_KILRATHI)
                send_message(victim, 0x1d);
            return 0;
        } else if (RandomBelowOrEqual(1) == 0)
            return 0;
    }
    if (g_aeSpecialManeuver_00495600[victim] ==
            SPECIAL_MANEUVER_UNKNOWN_9 &&
        g_aeObjectClass_00495328[victim] >= OBJECT_CLASS_SHIP)
        return 0;
    if (victim == 0) {
        if (g_bPlayerDamageEnabled_0049d77c == 0)
            return 0;
        else {
            g_bPlayerDestroyed_005d2fa4 = 1;
            if (g_bDeathSequenceActive_0049da50 == 0)
                g_nArcadeState_0049d75c = 4;
            return 1;
        }
    } else if (victim == g_nExternalViewShip_00493468)
        g_nExternalViewShip_00493468 = -1;
#ifdef SDL_PORT
    /* With no attacker, WC2 reads the zero-filled byte at 0x495207 just
     * before the owner table and consequently attributes the destruction to
     * object 0.  Preserve that result without crossing the native redzone. */
    if (attacker == -1)
        attacker = 0;
    else
        attacker = g_acObjectOwner_00495208[attacker];
#else
    if (g_acObjectOwner_00495208[attacker] == -1) {
    } else {
        attacker = g_acObjectOwner_00495208[attacker];
    }
#endif
    if (attacker != -1 &&
        g_aeObjectClass_00495328[victim] >= OBJECT_CLASS_SHIP) {
        analyze_kill(attacker, victim);
        for (objective = 0; objective < MISSION_OBJECTIVE_COUNT;
             objective++) {
            if (g_aMissionObjectives_004932a8[objective].type == 4 &&
                g_aMissionObjectives_004932a8[objective].index ==
                    g_asShipMissionIndex_00495d00[victim]) {
                g_aMissionObjectives_004932a8[objective].field_8 =
                    (unsigned char)g_asShipMissionIndex_00495d00[attacker];
            }
        }
    }
    if (victim < 10 && g_asShipSide_004955d0[victim] != SIDE_KILRATHI &&
        g_abShipEjectionSequenceEnabled_00496120[victim] != 0) {
        if (g_aeObjectClass_00495328[victim] >= OBJECT_CLASS_CAPITAL_SHIP)
            return 0;
        else if (g_aeObjectClass_00495328[victim] == OBJECT_CLASS_SHIP) {
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
    ComputeVectorDelta(&g_aShipPosition_00494550[obj], point,
                       &g_aShipVelocity_00494898[obj]);
    SetVectorFixedPoint((unsigned int *)&g_aShipVelocity_00494898[obj],
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
        if ((short)g_acObjectOwner_00495208[obj] == parent &&
            g_aeObjectClass_00495328[obj] == objectClass)
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
        if ((short)g_acObjectOwner_00495208[obj] == parent &&
            g_aeObjectClass_00495328[obj] == objectClass &&
            (target == -1 ||
             g_acShipTarget_00495f20[obj] == target))
            return obj;
    }
    return -1;
}

/* Function start: 0x414D3D */
int launch_object(short parent, short child, FixedVector direction,
                  short speed)
{
    NormalizeFixedVector(&direction);
    vector_component_in_dir(&g_aShipVelocity_00494898[parent],
                            &direction,
                            &g_aShipVelocity_00494898[child]);
    ScaleFixedVector(&direction, (int)speed << 8, &direction);
    AddFixedVectors(&direction, &g_aShipVelocity_00494898[child],
                    &g_aShipVelocity_00494898[child]);
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

    ready = g_asObjectCounter_00494be0[obj] <= 0;
    if (g_asShipFriendlyFireCooldown_00496090[obj] > 0)
        return;
    if (CountShipProjectiles(obj) > g_nAdaptiveDifficulty_005d3844)
        return;

    distance = g_nTargetRange_0049319c;
    if (RandomBelowOrEqual(0x13) == 0 &&
        RandomBelowOrEqual(7000) > distance)
        missileOk = 1;
    else
        missileOk = 0;
    if (missileOk != 0 &&
        g_aeObjectClass_00495328[target] >= OBJECT_CLASS_CAPITAL_SHIP &&
        g_nYourWingman_0049346c == obj &&
        (unsigned short)RandomInRange(0, 100) > 10)
        missileOk = 0;
    if (missileOk != 0 &&
        find_child_ship(obj, OBJECT_CLASS_MISSILE, target) != -1)
        missileOk = 0;
    if (g_bAiMissileFiringEnabled_00492d58 == 0)
        missileOk = 0;

    slot = 0;
    for (; slot < (signed char)g_aShipWeapons_004956b0[obj][0];
         slot++) {
        type = ((ShipWeaponSlot *)&g_aShipWeapons_004956b0[obj][1])[
                         slot].weaponType;
        stats = &g_aObjectTypeData_00496d30[type];
        type = ((ShipWeaponSlot *)&g_aShipWeapons_004956b0[obj][1])[
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
                    shouldFire = g_nFacingToTarget_00493194 > 70;
                    break;
                case 8:
                    shouldFire = g_nFacingToTarget_00493194 > 80;
                    break;
                case 10:
                    shouldFire = g_nFacingToTarget_00493194 > 80;
                    break;
                case 9:
                    shouldFire = g_nFacingToTarget_00493194 > 85;
                    break;
                case 13:
                    shouldFire = g_nFacingToTarget_00493194 > 55;
                    break;
                case 12:
                    shouldFire = g_nFacingToTarget_00493194 > 10;
                    break;
                }
            }
            if (shouldFire != 0)
                ((ShipWeaponSlot *)&g_aShipWeapons_004956b0[obj][1])[
                    slot].disabled = 0;
            else
                ((ShipWeaponSlot *)&g_aShipWeapons_004956b0[obj][1])[
                    slot].disabled = 1;
            if (shouldFire != 0) {
                firedObject = fire_weapon(obj, slot);
                if (g_anShipCloakState_00496020[obj] == 1)
                    ResetShipCloakState(obj);
                if (type == 12) {
                    ComputeVectorDelta(
                        &g_aShipPosition_00494550[firedObject],
                        &g_aShipPosition_00494550[target], &direction);
                    launch_object(obj, firedObject, direction,
                                  real_velocity(firedObject));
                }
            }
            break;

        case OBJECT_CLASS_MISSILE:
            if (missileOk != 0 && reachable != 0) {
                switch (type) {
                case 0x0f:
                    shouldFire = g_nFacingToTarget_00493194 > 0x61;
                    if (CanShipWeaponDamageTarget(obj, target) == 0)
                        shouldFire = 0;
                    if (shouldFire != 0 &&
                        g_asShipSide_004955d0[obj] == SIDE_KILRATHI) {
                        nearestRange = g_nTargetRange_0049319c;
                        other = 1;
                        for (; other < 10; other++) {
                            if (g_aeObjectClass_00495328[other] ==
                                    OBJECT_CLASS_SHIP &&
                                g_asShipSide_004955d0[other] ==
                                    SIDE_KILRATHI) {
                                get_facing_range_from_object(obj, other);
                                if (g_nFacingToTarget_00493194 > 0x5f &&
                                    g_nTargetRange_0049319c < nearestRange) {
                                    shouldFire = 0;
                                    break;
                                }
                            }
                        }
                        get_facing_range_from_object(obj, target);
                    }
                    break;
                case 0x10:
                    shouldFire = g_nFacingToTarget_00493194 > 0x28 &&
                                 g_nTargetFacing_00493198 < -0x3c;
                    if (CanShipWeaponDamageTarget(obj, target) == 0)
                        shouldFire = 0;
                    break;
                case 0x12:
                    shouldFire = g_nFacingToTarget_00493194 > 0x28;
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
                        g_nFacingToTarget_00493194 > 0x50)
                        shouldFire = 1;
                    else
                        shouldFire = 0;
                    break;
                }
            }
            if (shouldFire != 0) {
                if (g_anShipCloakState_00496020[obj] == 1)
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

    projectileVelocity = g_aObjectTypeData_00496d30[0x0c].maximumVelocity;
    set_objects_data(explosion, 0x0c, owner, 0);
    if (burstMode == 0) {
        lifetime = (short)(range / projectileVelocity - 5 -
                           RandomBelowOrEqual(8));
        g_asObjectCounter_00494be0[explosion] =
            MinShort(55, MaxShort(5, lifetime));
    } else {
        projectileVelocity = 220;
        g_asObjectCounter_00494be0[explosion] =
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

    target = g_acShipTarget_00495f20[obj];
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
        AddFixedVectors(&g_aShipPosition_00494550[target], &randomAim,
                        &burstPoint);
        rollPercent = RandomBelowOrEqual(100);
        if (burstMode != 0) {
            if (g_asObjectCounter_00494be0[obj] <= 0) {
                g_aShipPosition_00494550[burstObject] = *hardpoint;
                predictionSteps = (short)(range / 70);
                if (predictionSteps < 0)
                    predictionSteps = 0;
                if (predictionSteps > 20)
                    predictionSteps = 20;
                ScaleFixedVector(&g_aShipVelocity_00494898[target],
                                 predictionSteps << 8, &targetMotion);
                AddFixedVectors(&targetMotion, &burstPoint, &burstPoint);
                fire_flack(obj, burstObject, range, &burstPoint, burstMode);
                g_asObjectCounter_00494be0[obj] =
                    (short)(RandomBelow(10) + 7);
            }
        } else if ((g_asObjectCounter_00494be0[obj] <= 0 && rollPercent < 40) ||
                   rollPercent < 8) {
            g_aShipPosition_00494550[burstObject] = *hardpoint;
            fire_flack(obj, burstObject, range, &burstPoint, burstMode);
            g_asObjectCounter_00494be0[obj] =
                (short)(RandomBelow(10) + 7);
        } else if (g_asObjectType_00495298[
                       (signed char)g_acShipTarget_00495f20[obj]] != 0x13) {
            g_aShipPosition_00494550[burstObject] = burstPoint;
            explosion_shock_wave(
                burstObject,
                g_aObjectTypeData_00496d30[0x0c].explosionDamage);
        }
    }
    return burstObject;
}

/* Function start: 0x41596F */
short HasShipActiveMissile(short obj)
{
    short other;

    for (other = 0; other < 10; other++) {
        if (g_aeObjectClass_00495328[other] == OBJECT_CLASS_MISSILE &&
            g_acObjectOwner_00495208[other] == obj)
            return 1;
    }
    return 0;
}

/* Function start: 0x4159DB */
short fire_turrets(short obj)
{
    FixedVector hardpoint;
    short turretCount;
    short startTarget;
    short weapon;
    short targetHemisphere;
    short targetIndex;
    short targetRange;
    short targetCount;
    short other;
    short target;
    ShipWeaponSlot *slot;

    turretCount = 0;
    targetRange = 10000;
    if (g_asShipSide_004955d0[obj] == SIDE_IMPERIAL)
        targetRange = 5000;
    targetCount = build_target_list(obj, targetRange);
    for (weapon = 0;
         weapon < (signed char)g_aShipWeapons_004956b0[obj][0];
#ifdef SDL_PORT
         /* WC2 walks the slot pointer forward here before anything sets it -
          * this loop indexes the array directly and the loop below starts the
          * pointer over - and advancing an indeterminate pointer is undefined
          * behaviour the sanitizers catch.  Leave it alone on the port. */
         weapon++) {
#else
         weapon++, slot++) {
#endif
        if (((ShipWeaponSlot *)&g_aShipWeapons_004956b0[obj][1])[
                weapon].type == 0x0c)
            turretCount++;
    }

    slot = (ShipWeaponSlot *)&g_aShipWeapons_004956b0[obj][1];
    for (weapon = 0;
         weapon < (signed char)g_aShipWeapons_004956b0[obj][0];
         weapon++, slot++) {
        if (slot->type == 0x0d || slot->type == 0x0e)
            continue;
        if ((g_asShipSide_004955d0[obj] == SIDE_KILRATHI &&
             (unsigned short)RandomInRange(0, 200) <
                 GetAdaptiveTurnRate()) ||
            (g_asShipSide_004955d0[obj] == SIDE_IMPERIAL &&
             RandomBelowOrEqual(2) == 0)) {
            position_child(obj, weapon, &hardpoint);
            if (slot->type == 0x0c) {
                for (other = 1; other < 10; other++) {
                    if (g_aeObjectClass_00495328[other] !=
                            OBJECT_CLASS_NULL &&
                        g_asObjectType_00495298[other] == 0x13 &&
                        g_acShipTarget_00495f20[other] == obj) {
                        targetHemisphere = hemisphere(
                            &g_aShipPosition_00494550[other],
                            &g_aShipPosition_00494550[obj], &hardpoint);
                        if (g_asMissileProximityFuse_005d38e0[other] != 0 &&
                            RandomBelowOrEqual(
                                (short)(turretCount * 4 - 1)) == 0) {
                            g_acShipTarget_00495f20[obj] =
                                (signed char)other;
                            g_asObjectCounter_00494be0[obj] = 0;
                            SpawnFlakBurst(
                                obj, distance_from_object(obj, other),
                                &hardpoint, 1);
                        }
                    }
                }
            }
            if (targetCount != 0) {
                startTarget = RandomBelowOrEqual(
                    (short)(targetCount - 1));
                targetIndex = startTarget;
                do {
                    target = (short)g_acShipList_00496148[targetIndex];
                    targetHemisphere = hemisphere(
                        &g_aShipPosition_00494550[target],
                        &g_aShipPosition_00494550[obj], &hardpoint);
                    if (slot->type != 0x0c &&
                        g_aeObjectClass_00495328[target] >=
                            OBJECT_CLASS_SHIP) {
                        if (targetHemisphere > 60 &&
                            RandomBelowOrEqual(
                                (short)(200 - GetAdaptiveTurnRate())) == 0 &&
                            HasShipActiveMissile(obj) == 0) {
                            if (slot->type == 0x13) {
                                if (IsCapitalShipObject(target) != 0) {
                                    g_acShipTarget_00495f20[obj] =
                                        (signed char)target;
                                    fire_weapon(obj, weapon);
                                }
                            } else {
                                g_acShipTarget_00495f20[obj] =
                                    (signed char)target;
                                fire_weapon(obj, weapon);
                            }
                        }
                        break;
                    }
                    if (RandomBelowOrEqual(2) == 0) {
                        g_acShipTarget_00495f20[obj] =
                            (signed char)target;
                        SpawnFlakBurst(
                            obj, g_asShipListRange_00496158[targetIndex],
                            &hardpoint, 0);
                        break;
                    }
                    targetIndex++;
                    if (targetIndex > targetCount - 1)
                        targetIndex = 0;
                } while (targetIndex != startTarget);
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
    typeCode = ((ShipWeaponSlot *)&g_aShipWeapons_004956b0[obj][1])[
                     weapon].type;
    weaponDataIndex =
        ((ShipWeaponSlot *)&g_aShipWeapons_004956b0[obj][1])[
            weapon].weaponType;
    weaponClass = g_aObjectTypeData_00496d30[typeCode].objectClass;
    if (typeCode == 0x0c) {
        typeCode = 7;
        weaponClass = 8;
    }
    if (weaponClass == OBJECT_CLASS_MISSILE) {
        if (get_ship_slot() == -1) {
            for (target = 1; target < 10; target++) {
                if (g_aeObjectClass_00495328[target] ==
                        OBJECT_CLASS_MISSILE &&
                    g_asObjectType_00495298[target] == 0x14 &&
                    g_acObjectOwner_00495208[target] == obj)
                    explode(target, target);
            }
        }
        missile = initialize_ship(typeCode, obj, 0);
    } else {
        missile = new_object(typeCode, obj);
    }
    if (missile != -1) {
        weaponData = &g_aObjectTypeData_00496d30[weaponDataIndex];
        copy_frame(obj, missile);
        if (weaponClass == OBJECT_CLASS_PROJECTILE) {
            g_asObjectDamage_00495178[missile] =
                weaponData->damageCapacity;
            projectileSpeed = g_aObjectTypeData_00496d30[
                g_acObjectType_00493980[missile]].maximumVelocity;
            g_asShipWeaponEnergy_00495590[obj] =
                (short)(g_asShipWeaponEnergy_00495590[obj] -
                        weaponData->animationDelay);
        }
        child_object(weapon, missile, obj);
        g_asObjectCounter_00494be0[missile] =
            *(int *)&g_aObjectTypeData_00496d30[
                weaponDataIndex].lifetime;
        vector_component_in_dir(&g_aShipVelocity_00494898[obj],
                                &g_aShipForwardVector_00494208[missile],
                                &g_aShipVelocity_00494898[missile]);
        if (weaponClass == OBJECT_CLASS_PROJECTILE) {
            if (obj == 0 && g_bTargetLockActive_0049ae80 != 0 &&
                g_bMissileLockAcquired_0049b2b0 != 0) {
                targetRange = distance_from_object(
                    0, g_acShipTarget_00495f20[0]);
                leadRange = (short)(targetRange + 100);
            } else {
                leadRange = (short)((*(int *)&weaponData->lifetime + 5) *
                                       weaponData->maximumVelocity);
            }
            ScaleFixedVector(&g_aShipForwardVector_00494208[obj],
                             leadRange << 8, &travel);
            AddFixedVectors(&g_aShipPosition_00494550[obj], &travel,
                            &travel);
            point_at(missile, travel);
        }
        ScaleFixedVector(&g_aShipForwardVector_00494208[missile],
                         (int)projectileSpeed << 8, &travel);
        AddFixedVectors(&travel, &g_aShipVelocity_00494898[missile],
                        &g_aShipVelocity_00494898[missile]);
        RecordCannedSceneObjectEvent(missile, 0);
        if (weaponClass == OBJECT_CLASS_MISSILE) {
            ScaleFixedVector(&g_aShipUpVector_00493ec0[obj], 0xa00,
                             &travel);
            AddFixedVectors(&travel, &g_aShipVelocity_00494898[missile],
                            &g_aShipVelocity_00494898[missile]);
            if (obj == 0 && typeCode != 0x14)
                RemovePlayerReleaseWeapon(weapon);
            else
                remove_weapon(obj, weapon);
            g_aeSpecialManeuver_00495600[missile] = -1;
            g_asShipManeuver_00495f48[missile] =
                g_aeSpecialManeuver_00495600[missile];
            g_asShipTactic_00495f30[missile] = 1;
            g_asPilotLevel_00495d60[missile] = -1;
            g_abShipEjectionSequenceEnabled_00496120[missile] = 0;
            g_asObjectCounter_00494be0[missile] = 5;
            g_acObjectCollisionGraceTicks_00494d48[missile] = 20;
            switch (typeCode) {
            case 0x0f:
                steady_object(missile);
                g_acShipTarget_00495f20[missile] =
                    g_acShipTarget_00495f20[obj];
                g_asObjectCounter_00494be0[missile] = 1;
                g_anShipSpeed_00494e20[missile] =
                    get_ship_max_velocity(missile) << 8;
                if (g_acShipTarget_00495f20[missile] != -1) {
                    ComputeVectorDelta(&g_aShipPosition_00494550[obj],
                        &g_aShipPosition_00494550[
                            g_acShipTarget_00495f20[missile]], &travel);
                    interceptScale =
                        Vector_magnitude(&travel) /
                        get_ship_max_velocity(missile);
                    ScaleFixedVector(&g_aShipVelocity_00494898[
                        g_acShipTarget_00495f20[missile]],
                        interceptScale, &travel);
                    AddFixedVectors(&g_aShipPosition_00494550[
                        g_acShipTarget_00495f20[missile]], &travel,
                        &travel);
                    point_at(missile, travel);
                }
                break;
            case 0x10:
            case 0x12:
                g_acShipTarget_00495f20[missile] =
                    g_acShipTarget_00495f20[obj];
                break;
            case 0x13:
                g_acShipTarget_00495f20[missile] =
                    g_acShipTarget_00495f20[obj];
                if (g_nCurrentView_00492fa8 == 4)
                    g_acShipTarget_00495f20[missile] =
                        (signed char)g_nSavedPlayerTarget_0049d460;
                triggerDistance = (short)(distance_from_object(
                    obj, g_acShipTarget_00495f20[missile]) / 1000);
                if (triggerDistance > 9 ||
                    (unsigned short)RandomInRange(0, 15) <
                        triggerDistance)
                    g_asMissileProximityFuse_005d38e0[missile] = 1;
                else
                    g_asMissileProximityFuse_005d38e0[missile] = 0;
                break;
            case 0x11:
                g_asObjectCounter_00494be0[missile] = 15;
                g_acShipTarget_00495f20[missile] = -1;
                break;
            case 0x14:
                g_asObjectCounter_00494be0[missile] = 120;
                g_acShipTarget_00495f20[missile] = -1;
                ScaleFixedVector(&g_aShipForwardVector_00494208[obj],
                                 0xa00, &mountOffset);
                SubtractFixedVectors(&g_aShipVelocity_00494898[obj],
                                     &mountOffset,
                                     &g_aShipVelocity_00494898[missile]);
                break;
            }
        }
        if (obj == 0) {
            if (weaponClass == OBJECT_CLASS_PROJECTILE) {
                g_asObjectCounter_00494be0[obj] =
                    g_acGunRefireDelay_00492e1c[
                        typeCode - 7];
                if (g_asObjectType_00495298[0] == 0x33)
                    g_asObjectCounter_00494be0[obj] <<= 1;
            }
        } else if (obj == g_nYourWingman_0049346c) {
            g_asObjectCounter_00494be0[obj] =
                (short)(10 - (g_nAdaptiveDifficulty_005d3844 >> 1));
        } else {
            g_asObjectCounter_00494be0[obj] =
                (short)(14 - g_nAdaptiveDifficulty_005d3844);
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
         weapon < (signed char)g_aShipWeapons_004956b0[obj][0];
         weapon++) {
        if (((ShipWeaponSlot *)&g_aShipWeapons_004956b0[obj][1])[weapon].type ==
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

    index = (ShipWeaponSlot *)&g_aShipWeapons_004956b0[ship][1];
    for (weapon = 0;
         weapon < (signed char)g_aShipWeapons_004956b0[ship][0];
         weapon++, index++) {
        if (g_aObjectTypeData_00496d30[index->weaponType].objectClass ==
                OBJECT_CLASS_MISSILE) {
            if (ship != 0) {
                if (index->type != 0x13 ||
                    IsCapitalShipObject(g_acShipTarget_00495f20[ship]) != 0)
                    return fire_weapon(ship, weapon);
            } else if (index->disabled == 0) {
                if ((index->type == 0x10 || index->type == 0x12 ||
                     index->type == 0x13) &&
                    g_nTargetLockCountdown_004934ec != 0) {
                    if (get_mode(0) == 1)
                        ShowComponentHitHudMessage(
                            g_szNeedLock_00492e4c,
                            g_abGamePaletteReservedColours_0049cb54[4], 3);
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

    index = (ShipWeaponSlot *)&g_aShipWeapons_004956b0[obj][1];
    for (weapon = 0;
         weapon < (signed char)g_aShipWeapons_004956b0[obj][0];
         weapon++, index++) {
#ifdef SDL_PORT
        SdlTracef("[index] obj=%d w=%d type=%d wtype=%d class=%d dis=%d\n",
                     (int)obj, (int)weapon, (int)index->type,
                     (int)index->weaponType,
                     (int)g_aObjectTypeData_00496d30[
                         index->weaponType].objectClass,
                     (int)index->disabled);
#endif
        if (g_aObjectTypeData_00496d30[index->weaponType].objectClass ==
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
    if (g_asObjectType_00495298[obj] == 0x33)
        return;
    if (get_ship_max_velocity(obj) * 0x500 >
            Vector_magnitude(&g_aShipVelocity_00494898[obj])) {
        set_special(obj, SPECIAL_MANEUVER_AFTERBURNER);
        if (g_aeSpecialManeuver_00495600[obj] !=
            SPECIAL_MANEUVER_AFTERBURNER)
            time = 0;
        g_asShipAfterburnerTimer_004955a8[obj] = time;
    }
}

/* Function start: 0x4169CC */
void fire_super_brake(short ship)
{
    g_asShipAfterburnerTimer_004955a8[ship] = 10;
    set_special(ship, SPECIAL_MANEUVER_SUPER_BRAKE);
}

/* Function start: 0x4169F3 */
short flip_angle(short ship, short angle)
{
    if ((g_asObjectFlip_004939c8[ship] & 0x10) != 0)
        angle = (short)(180 - angle);
    if ((g_asObjectFlip_004939c8[ship] & 0x20) != 0)
        angle = (short)-angle;
    angle += g_asObjectScreenAngle_004936b8[ship];
    angle %= 360;
    if (angle < 0)
        angle += 360;
    return angle;
}

/* Function start: 0x416A87 */
void place_exhaust_on_ships(void)
{
    short *animation;
    short frame;
    short scale;
    short object;
    short ship;
    short viewFrame;
    short animationOffset;

    for (ship = 0; ship < 10; ship++) {
        if (g_aeObjectClass_00495328[ship] < OBJECT_CLASS_MISSILE ||
            g_anShipSpeed_00494e20[ship] == 0 ||
            g_asObjectType_00495298[ship] == 0x14 ||
            g_aeSpecialManeuver_00495600[ship] ==
                SPECIAL_MANEUVER_UNKNOWN_9 ||
            g_aeSpecialManeuver_00495600[ship] ==
                SPECIAL_MANEUVER_KILL_ENGINES ||
            g_anShipCloakState_00496020[ship] == 1)
            continue;
        if (ship == 0 && g_bJumpSequenceActive_004962f0 != 0)
            continue;
        if (g_asObjectScreenX_00493598[ship] == (short)0x8001)
            continue;

        animation = (short *)g_apObjectExhaustShape_004953b8[ship];
        if (animation == 0)
            continue;
        viewFrame = g_asObjectViewFrame_00493508[ship];
#ifdef SDL_PORT
        /* These ships load each view as a separate, single-frame packet.
         * Retail selects the loaded view's exhaust records, not frame zero
         * within that packet. Otherwise the engines detach as the view turns. */
        if (g_aeObjectClass_00495328[ship] >= OBJECT_CLASS_CAPITAL_SHIP ||
            g_asObjectType_00495298[ship] == 0x33 ||
            (g_bExpandedShipGraphicsEnabled_004931a4 != 0 &&
             g_aeObjectClass_00495328[ship] == OBJECT_CLASS_SHIP))
#else
        if (g_aeObjectClass_00495328[ship] < OBJECT_CLASS_CAPITAL_SHIP &&
            g_asObjectType_00495298[ship] != 0x33 &&
            g_bExpandedShipGraphicsEnabled_004931a4 != 0 &&
            g_aeObjectClass_00495328[ship] == OBJECT_CLASS_SHIP)
#endif
            viewFrame = g_asLoadedShipViewFrame_00495d18[ship];
        animationOffset = animation[viewFrame];
        if (animationOffset == -1)
            continue;
        animation =
            (short *)((unsigned char *)animation + animationOffset);
        if (*animation == -1)
            continue;

        while (*animation != -1) {
            object = find_vacant_3d_object();
            if (object == -1)
                return;
            set_objects_data(object, 0x2b, ship, 0);
            frame = *animation++;
            scale = *animation++;
            if (g_aeObjectClass_00495328[ship] >=
                    OBJECT_CLASS_CAPITAL_SHIP)
                scale = (short)(scale +
                    (unsigned short)RandomInRange(0x10, 0x30));
            else
                scale = (short)(scale +
                    (unsigned short)RandomInRange(0, 0x20));
            if (g_acShipExhaustHeat_00495660[ship] == 0)
                scale = (short)(scale - 0x20);
            g_asObjectScale_00494d90[object] = scale;
            g_asObjectDistance_00493ae8[object] = *animation++;
            g_asObjectScreenAngle_004936b8[object] =
                flip_angle(ship, *animation++);
            g_asObjectFlip_004939c8[object] = 0;
            g_asObjectScreenX_00493598[object] = *animation++;
            g_asObjectScreenY_00493628[object] = *animation++;
            if (g_aeSpecialManeuver_00495600[ship] ==
                    SPECIAL_MANEUVER_AFTERBURNER) {
                g_asObjectViewFrame_00493508[object] =
                    (short)(frame * 3 +
                            (unsigned short)RandomInRange(0, 2));
            } else {
                g_asObjectViewFrame_00493508[object] =
                    (short)((unsigned short)RandomInRange(0, 1) +
                            frame * 2 + 12);
            }
        }
    }
}

/* Function start: 0x416DC3 */
void reposition_fixed_child_objects(void)
{
    int right;
    short child;
    int up;
    long cosA;
    long sine;
    short angle;
    short owner;
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
        if (g_aeObjectClass_00495328[child] ==
            OBJECT_CLASS_FIXED_OBJECT) {
            owner = (short)g_acObjectOwner_00495208[child];
            switch (g_asObjectType_00495298[child]) {
            case 0x3e:
                g_asObjectDistance_00493ae8[child] =
                    (short)((unsigned short)
                                g_asObjectDistance_00493ae8[owner] -
                            100);
                g_asObjectScreenX_00493598[child] =
                    g_asObjectScreenX_00493598[owner];
                g_asObjectScreenY_00493628[child] =
                    g_asObjectScreenY_00493628[owner];
                break;
            case 0x3f:
                g_asObjectDistance_00493ae8[child] =
                    (short)((unsigned short)
                                g_asObjectDistance_00493ae8[owner] +
                            100);
                g_asObjectScreenX_00493598[child] =
                    g_asObjectScreenX_00493598[owner];
                g_asObjectScreenY_00493628[child] =
                    g_asObjectScreenY_00493628[owner];
                break;
            case 0x0c:
            case 0x2b:
                angle = g_asObjectScreenAngle_004936b8[owner];
                sine = SinFixed(angle);
                cosA = CosFixed(angle);
                g_asObjectDistance_00493ae8[child] =
                    (short)((unsigned short)
                                g_asObjectDistance_00493ae8[owner] +
                            (unsigned short)
                                g_asObjectDistance_00493ae8[child]);
                right = g_asObjectScreenX_00493598[child];
                right *= (unsigned short)
                    g_asObjectScreenScale_00493a58[owner];
                if ((g_asObjectFlip_004939c8[owner] & 0x10) != 0)
                    right = -right;
                up = g_asObjectScreenY_00493628[child];
                up *= (unsigned short)
                    g_asObjectScreenScale_00493a58[owner];
                if ((g_asObjectFlip_004939c8[owner] & 0x20) != 0)
                    up = -up;
                g_asObjectScreenX_00493598[child] = (short)(
                    (MultiplyFixed(right, (int)cosA) -
                     MultiplyFixed(up, (int)sine)) >> 8);
                g_asObjectScreenY_00493628[child] = (short)(
                    (MultiplyFixed(up, (int)cosA) +
                     MultiplyFixed(right, (int)sine)) >> 8);
                g_asObjectScreenX_00493598[child] +=
                    g_asObjectScreenX_00493598[owner];
                g_asObjectScreenY_00493628[child] +=
                    g_asObjectScreenY_00493628[owner];
#ifdef SDL_PORT
                if (g_asObjectType_00495298[child] == 0x2b) {
                    /* Match the anchor to the enhanced owner transform. */
                    parentScreenX =
                        (float)g_nViewCenterX_005c80d8 +
                        (float)(((double)(g_nScreenWidth_0049d4d8 & ~1) *
                                 0.5 *
                                 g_aObjectViewPosition_0059afa0[owner].x) /
                                g_aObjectViewPosition_0059afa0[owner].z);
                    parentScreenY =
                        (float)g_nViewCenterY_005c80da +
                        (float)(((double)(g_nScreenWidth_0049d4d8 & ~1) *
                                 0.5 *
                                 g_aObjectViewPosition_0059afa0[owner].y) /
                                g_aObjectViewPosition_0059afa0[owner].z);
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
            g_asObjectScreenScale_00493a58[child] = (short)(
                (unsigned short)g_asObjectScale_00494d90[child] *
                (unsigned short)g_asObjectScreenScale_00493a58[owner] >>
                8);
        }
    }
}

/* Function start: 0x4170F9 */
void housekeep_power_plant_and_fuel(short ship)
{
    if (0 < g_anShipSpeed_00494e20[ship])
        drain_fuel(ship, 5);
}

void PlaySfxWaveFileByNumber(int soundNumber, int sourceObject, int looping);
short MinShort(short a, short b);

/* Function start: 0x417124 */
void replenish_shields(short ship)
{
    signed char shield;

    if (ship == 0 && g_acPlayerComponentDamage_00493470[1] > 0 &&
        g_nSpaceFrame_00493134 %
            (g_acPlayerComponentDamage_00493470[1] + 1) != 0)
        return;
    for (shield = 0; shield <= 1; shield++) {
        if (g_aasShipMaximumShield_004954f0[ship][shield] <
            g_aasShipShield_00495518[ship][shield])
            g_aasShipShield_00495518[ship][shield] =
                g_aasShipMaximumShield_004954f0[ship][shield];
        if (g_aasShipMaximumShield_004954f0[ship][shield] >
                g_aasShipShield_00495518[ship][shield] &&
            g_nSpaceFrame_00493134 %
                g_aObjectTypeData_00496d30[
                    g_acObjectType_00493980[ship]].animationDelay == 0) {
            g_aasShipShield_00495518[ship][shield]++;
        }
    }
}

/* Function start: 0x41724A */
void replenish_weapon_energy_bank(short ship)
{
    short maximumShield;
    short shieldEnergy;

    if (ship == 0 && g_acPlayerComponentDamage_00493470[1] != 0 &&
        (int)(unsigned short)RandomInRange(0, 4) <
            g_acPlayerComponentDamage_00493470[1])
        return;
    if (ship == 0 && HasShipCockpitGunDisplay(0) != 0) {
        if (g_nTargetCameraMode_005c8d50 == 0) {
            g_nGunDisplayEnergyPercent_005c8d4e = MinShort(
                (short)(g_nGunDisplayEnergyPercent_005c8d4e + 2), 100);
        } else if (g_nTargetCameraZoom_0049d3e4 < 0x21 &&
                   g_nSpaceFrame_00493134 % 2 == 0) {
            g_nTargetCameraZoom_0049d3e4 = MinShort(
                (short)(g_nTargetCameraZoom_0049d3e4 + 1), 0x21);
        }
    }
    if (g_asShipWeaponEnergy_00495590[ship] < 100) {
        shieldEnergy = (short)(g_aasShipShield_00495518[ship][1] +
                               g_aasShipShield_00495518[ship][0]);
        maximumShield =
            (short)(g_aasShipMaximumShield_004954f0[ship][1] +
                    g_aasShipMaximumShield_004954f0[ship][0]);
        if (maximumShield > shieldEnergy) {
            g_asShipWeaponEnergy_00495590[ship] =
                MinShort(
                    (short)(g_asShipWeaponEnergy_00495590[ship] + 1),
                    100);
        } else {
            g_asShipWeaponEnergy_00495590[ship] =
                MinShort(
                    (short)(g_asShipWeaponEnergy_00495590[ship] + 2),
                    100);
        }
        if (g_asShipWeaponEnergy_00495590[ship] < -5) {
            g_asShipWeaponEnergy_00495590[ship] = MinShort(
                (short)(g_asShipWeaponEnergy_00495590[ship] + 1), 100);
        }
    }
}

/* Function start: 0x41740F */
void accelerate(short amount)
{
    if (malf(0) != 0) {
        amount = (short)(amount - 2);
        if (g_nSpaceFrame_00493134 % 3 == 0)
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

    if (g_anShipFuel_00495638[0] > 0) {
        if (malf(0) != 0) {
            PlaySfxWaveFileByNumber(3, -1, 0);
        } else {
            if (g_aeSpecialManeuver_00495600[0] !=
                    SPECIAL_MANEUVER_AFTERBURNER) {
                time = 8;
            } else if (g_asShipAfterburnerTimer_004955a8[0] == 0) {
                time = 8;
            } else {
                if (g_asShipAfterburnerTimer_004955a8[0] > 2)
                    return;
                else
                    time = 2;
            }
            fire_afterburner(0, time);
            if (g_nSpaceFrame_00493134 >
                g_nNextAfterburnerSfxFrame_005d3820) {
                g_nNextAfterburnerSfxFrame_005d3820 =
                    (int)g_nSpaceFrame_00493134 + 6;
                PlaySfxWaveFileByNumber(12, -1, 0);
            }
        }
    }
}
