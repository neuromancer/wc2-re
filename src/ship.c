/*
 *  Ship object, damage, explosion, and weapon logic from the Mac `ship`
 *  compilation unit.
 *
 *  Address range 0x41f000-0x420fff (provisional -- see docs/ORDER.md).
 *  Boundary evidence: the ordered Mac `ship` symbols map across the larger
 *  0x41dee0-0x42193f Win32 run; this file currently owns its central tranche.
 */
#include "wc1.h"

static const short g_aaeExplosionDebris_00492dd8[4][7] = {
    {
        WC2_OBJECT_TYPE_PIPE, WC2_OBJECT_TYPE_O_RING,
        WC2_OBJECT_TYPE_GIRDER_CHUNK, WC2_OBJECT_TYPE_SHIP_TUBING,
        WC2_OBJECT_TYPE_METAL_SHEET, WC2_OBJECT_TYPE_SHIP_WING,
        WC2_OBJECT_TYPE_BURNING_DEBRIS
    },
    {
        WC2_OBJECT_TYPE_O_RING, WC2_OBJECT_TYPE_O_RING,
        WC2_OBJECT_TYPE_GIRDER_CHUNK, WC2_OBJECT_TYPE_SHIP_TUBING,
        WC2_OBJECT_TYPE_SHIP_WING, WC2_OBJECT_TYPE_GIRDER_CHUNK,
        WC2_OBJECT_TYPE_BURNING_DEBRIS
    },
    {
        WC2_OBJECT_TYPE_PIPE, WC2_OBJECT_TYPE_O_RING,
        WC2_OBJECT_TYPE_METAL_SHEET, WC2_OBJECT_TYPE_SHIP_TUBING,
        WC2_OBJECT_TYPE_METAL_SHEET, WC2_OBJECT_TYPE_SHIP_WING,
        WC2_OBJECT_TYPE_SHIP_TUBING
    },
    {
        WC2_OBJECT_TYPE_BURNING_DEBRIS, WC2_OBJECT_TYPE_SHIP_TUBING,
        WC2_OBJECT_TYPE_METAL_SHEET, WC2_OBJECT_TYPE_SHIP_WING,
        WC2_OBJECT_TYPE_PIPE, WC2_OBJECT_TYPE_O_RING,
        WC2_OBJECT_TYPE_BURNING_DEBRIS
    }
};

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
    int sideDot;
    short ship;
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
        if (victim == 0)
            TriggerPlayerHitPaletteFlash();
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
                    for (ship = 1; ship < 10; ship++) {
                        if (g_asShipSide_004955d0[ship] == SIDE_IMPERIAL) {
                            if (g_aeObjectClass_00495328[ship] ==
                                OBJECT_CLASS_SHIP) {
                                g_asShipMissionType_00495de8[ship] =
                                    MISSION_TYPE_STRIKE;
                                g_asShipMissionParameter_00495e00[ship] =
                                    g_asShipMissionIndex_00495d00[0];
                                g_aeShipObjective_00495f08[ship] =
                                    OBJECTIVE_DESTROY_SHIP;
                                g_acShipTarget_00495f20[ship] = 0;
                            } else if (g_aeObjectClass_00495328[ship] >=
                                       OBJECT_CLASS_CAPITAL_SHIP) {
                                g_asShipMissionType_00495de8[ship] =
                                    MISSION_TYPE_NONE;
                                g_asShipTactic_00495f30[ship] =
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
        sideDot = dot_product(
            impactDirection, &g_aShipRightVector_00493b78[victim]);
        if (sideDot > 0xb5)
            quadrant = 3;
        else if (sideDot < -0xb5)
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
                if (g_bPlayerDamageEnabled_0049d77c != 0)
                    g_nArcadeState_0049d75c = 4;
                return -1;
            }
        } else if (skill_check(obj) == 0 && obj != 0) {
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
    short maximum;

    g_aasShipMaximumShield_004954f0[obj][0] =
        MaxShort(0, (short)(g_aasShipMaximumShield_004954f0[obj][0] -
            (g_aObjectTypeData_00496d30[
                g_acObjectType_00493980[obj]].shieldFore >> 2)));
    maximum = g_aasShipMaximumShield_004954f0[obj][0];
    g_aasShipMaximumShield_004954f0[obj][1] =
        MaxShort(0, (short)(maximum -
            (g_aObjectTypeData_00496d30[
                g_acObjectType_00493980[obj]].shieldAft >> 2)));
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
        if (otherGuns == 0)
            return 1;
        return 0;
    }
    return 0;
}

/* Function start: 0x413099 */
short your_internal_damage(short attacker, short owner, short damage,
                           short quadrant)
{
    signed char system;
    signed char severity;
    short tableGroup;
    signed char weapon;
    short playerType;

    if (g_aeObjectClass_00495328[attacker] ==
        OBJECT_CLASS_PROJECTILE) {
        if (quadrant != 1)
            tableGroup = 0;
        else
            tableGroup = 2;
        damage = MaxShort(1, (short)(damage >> 4));
    } else if (g_aeObjectClass_00495328[attacker] ==
                   OBJECT_CLASS_ASTEROID ||
               g_aeObjectClass_00495328[attacker] >=
                   OBJECT_CLASS_SHIP) {
        tableGroup = 4;
        damage = MaxShort(1, (short)(damage >> 7));
    } else {
        if (quadrant != 1)
            tableGroup = 1;
        else
            tableGroup = 3;
        damage = MaxShort(1, (short)(damage >> 5));
    }
    playerType = (short)g_acObjectType_00493980[0];
    severity = (signed char)RandomBelowOrEqual(10);
    g_asObjectDamage_00495178[0] =
        (short)(g_asObjectDamage_00495178[0] + damage);
    if (damage > 1) {
        DecreaseAdaptiveDifficulty();
        place_damage_on_cockpit(RandomBelowOrEqual(3));
    }

    while (damage > 0) {
        damage--;
        system = *(const signed char *)(const void *)
            &g_asPlayerDamageSystemTable_00492d70[
                tableGroup * 10 + RandomBelowOrEqual(9)];
        switch (system) {
        case 8:
            if (quadrant != 0) {
                damage++;
            } else if (severity < 7) {
                damage_your_component(4, 2, 3);
                if (g_acPlayerComponentDamage_00493470[4] >= 4)
                    g_acShipDestroyedWeaponCount_00495680[0] = -1;
            } else {
                damage_your_component(5, 4, 4);
            }
            break;
        case 0:
            if (severity < 4) {
                pilot_hit(0);
            } else if (severity < 7) {
                damage_your_component(7, 2, 4);
            } else {
                damage_your_component(6, 4, 4);
            }
            break;
        case 6:
            if (quadrant != 0) {
                damage++;
            } else if (g_acShipCommunicator_00495670[0] < 5) {
                g_acShipCommunicator_00495670[0]++;
                check_computer_damage();
            }
            break;
        case 5:
            if (quadrant != 0) {
                damage++;
            } else if ((signed char)g_aShipWeapons_004956b0[0][0] > 1) {
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
            } else {
                if (RandomBelowOrEqual(3) == 0)
                    return explode(attacker, 0);
                if (damage_your_component(1, 1, 4) == 4)
                    return explode(attacker, 0);
            }
            break;
        case 3:
            if (severity < 9) {
                damage_your_component(2, 1, 4);
                revise_shields(0);
            } else {
                damage_your_component(8, 2, 4);
            }
            break;
        case 4:
            g_acShipDamage_00495690[0]++;
            if (g_acShipDamage_00495690[0] == 1) {
                if (pilot_hit(0) != 0)
                    damage--;
            } else if ((short)g_acShipDamage_00495690[0] >
                       g_aObjectTypeData_00496d30[playerType].
                           damageCapacity) {
                return explode(attacker, 0);
            }
            break;
        case 7:
            drain_fuel(0,
                       (short)(*(int *)&g_aObjectTypeData_00496d30[
                                   playerType].lifetime / 4));
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
            } else {
                damage_your_component(0, 1, 3);
                damage_ion_drive(0, 1, 3);
            }
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
    }
    return 0;
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

/* Function start: 0x4136E2 */
void Create_ship_hit_debris(short obj, short count)
{
    FixedVector offset;
    short debris;
    short created;

    created = 0;
    while (created < count) {
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
        created++;
    }
}

/* Function start: 0x4137C2 */
void check_next_wave(void)
{
    {
        short obj;

        for (obj = 0; obj < 10; obj++) {
            if (g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_SHIP &&
                g_asShipSide_004955d0[obj] == SIDE_KILRATHI)
                return;
        }
    }
    if (g_nEscapedEnemyCount_004962e8 != 0)
        return;
    {
        MissionObjective *objectiveData;
        short objective;

        for (objective = 0;
             objective < (short)g_cMissionObjectiveCount_00493294;
             objective++) {
            objectiveData = &g_aMissionObjectives_004932a8[objective];
            if (objectiveData->type == 5 &&
                objectiveData->index == g_nCurrentNavPoint_004931bc) {
                flag_objective(objective, 2);
                return;
            }
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
    if (set_up_next_wave() != 0)
        return;
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
        ejection_sequence((short)(g_nPlayerShipType_00493464 + 300), 1);
        g_bMissionDeathSequencePending_0049b720 = 0;
        g_bMissionEjectionPodSpawned_0049b724 = 0;
    }
}

/* Function start: 0x413A3B */
unsigned int Create_explosion_debris(short obj)
{
    FixedVector vector;
    short debris;
    short index;
    short set;

    remove_object(obj);
    index = 0;
    check_next_wave();
    set = RandomBelowOrEqual(3);
    for (; index < 7; index++) {
        debris = find_vacant_3d_object();
        if (debris == -1)
            break;
        set_objects_data(debris,
                         g_aaeExplosionDebris_00492dd8[set][index], -1, 0);
        g_asObjectCounter_00494be0[debris] = 40;
        FillFixedVectorWithRandomComponents(50, &vector);
        AddFixedVectors(&g_aShipPosition_00494550[obj], &vector,
                        &g_aShipPosition_00494550[debris]);
        FillFixedVectorWithRandomComponents(
            25, &g_aShipVelocity_00494898[debris]);
        divide_vector(&g_aShipVelocity_00494898[obj], 0x200, &vector);
        AddFixedVectors(&vector,
                        &g_aShipVelocity_00494898[debris],
                        &g_aShipVelocity_00494898[debris]);
    }
    index = 0;
    for (; index < 8; index++) {
        debris = find_vacant_3d_object();
        if (debris == -1)
            break;
        FillFixedVectorWithRandomComponents(50, &vector);
        AddFixedVectors(&g_aShipPosition_00494550[obj], &vector,
                        &g_aShipPosition_00494550[debris]);
        FillFixedVectorWithRandomComponents(
            25, &g_aShipVelocity_00494898[debris]);
        divide_vector(&g_aShipVelocity_00494898[obj], 0x200, &vector);
        AddFixedVectors(&vector,
                        &g_aShipVelocity_00494898[debris],
                        &g_aShipVelocity_00494898[debris]);
        g_asObjectScreenAngle_004936b8[debris] =
            (short)(RandomBelowOrEqual(3) + 0x10);
        g_asObjectCounter_00494be0[debris] = 40;
        g_aeObjectClass_00495328[debris] = OBJECT_CLASS_DUST;
        g_acObjectType_00493980[debris] = OBJECT_TYPE_DEBRIS_DUST;
    }
    return 0;
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
unsigned int score_for_kill(short pilot, short victim)
{
    int event;

    if (g_asShipSide_004955d0[victim] == SIDE_KILRATHI) {
        switch (g_acObjectType_00493980[victim]) {
        case OBJECT_TYPE_SALTHI:
            event = 1;
            break;
        case OBJECT_TYPE_DRALTHI:
        case OBJECT_TYPE_KRANT:
            event = 2;
            break;
        case OBJECT_TYPE_GRATHA:
        case OBJECT_TYPE_JALTHI:
            event = 3;
            break;
        case OBJECT_TYPE_SPIKERI:
        case OBJECT_TYPE_RALARI:
            event = 6;
            break;
        case OBJECT_TYPE_DORKIR:
        case OBJECT_TYPE_LUMBARI:
            event = 4;
            break;
        case OBJECT_TYPE_FRALTHI:
        case OBJECT_TYPE_SNAKEIR:
            event = 7;
            break;
        case OBJECT_TYPE_SIVAR:
        case OBJECT_TYPE_KILRATHI_BASE:
            event = 8;
            break;
        }
        affect_mission_score(pilot, event, -1);
    }
    return 0;
}

/* Function start: 0x413F58 */
void analyze_kill(short attacker, short victim)
{
    short enemy;
    short initialShip;

    enemy = g_asShipSide_004955d0[attacker] !=
            g_asShipSide_004955d0[victim];
    new_space_music_changes(attacker, victim);
    if (enemy != 0) {
        score_for_kill(attacker, victim);
        if (attacker == 0) {
            g_cPlayerKillCount_005d2fa8++;
        } else {
            initialShip = 0;
            while (initialShip < 8) {
                if (g_stMissionHeader_005d3e70
                        .initialMissionShips[initialShip] ==
                    g_asShipMissionIndex_00495d00[attacker])
                    g_acInitialShipKillCount_005d2fc0[initialShip]++;
                initialShip++;
            }
        }
    }
}

/* Function start: 0x4142C8 */
short ShipExplosion(short obj)
{
    unsigned short originalScale;
    short explosion;

    originalScale = (unsigned short)g_asObjectScale_00494d90[obj];
    explosion = find_vacant_3d_object();
    if (explosion == -1) {
        if (g_aeObjectClass_00495328[obj] ==
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
    set_objects_data(explosion, OBJECT_TYPE_EXPLOSION1,
                     (short)g_acObjectOwner_00495208[explosion], 0);
    g_asObjectScale_00494d90[explosion] = (short)(
        (unsigned short)g_asObjectScale_00494d90[explosion] *
        originalScale >> 8);
    return explosion;
}

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
                WC2_OBJECT_TYPE_EXPLOSION_LARGE].shapeSet != 0)
            g_asObjectCounter_00494be0[obj] = 8;
        else
            g_asObjectCounter_00494be0[obj] = 4;
        if (g_aObjectTypeData_00496d30[
                WC2_OBJECT_TYPE_EXPLOSION_LARGE].shapeSet != 0)
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
                     WC2_OBJECT_TYPE_EXPLOSION_LARGE].shapeSet != 0 ? 8 : 4));
            g_asShipExplosionStageTimer_005d3850[obj] = (short)(
                (g_aObjectTypeData_00496d30[
                     WC2_OBJECT_TYPE_EXPLOSION_LARGE].shapeSet != 0 ?
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
        if (g_asObjectType_00495298[obj] == WC2_OBJECT_TYPE_TORPEDO) {
            debris = obj;
            set_objects_data(debris, WC2_OBJECT_TYPE_EXPLOSION_LARGE,
                             g_acObjectOwner_00495208[obj], 0);
        } else {
            debris = obj;
            set_objects_data(debris, WC2_OBJECT_TYPE_EXPLOSION_SMALL,
                             g_acObjectOwner_00495208[obj], 0);
        }
        RecordCannedSceneObjectEvent(debris, 0);
    }
    if (g_asObjectScreenX_00493598[obj] != -32767)
        PlaySfxWaveFileByNumber(4, obj, 0);
    return debris;
}

/* Function start: 0x414025 */
void explosion_shock_wave(short obj, short blastDamage)
{
    FixedVector force;
    short distance;
    short damage;
    FixedVector delta;
    short target;

    for (target = 0; target < 10; target++) {
        if (obj != target &&
            g_aeObjectClass_00495328[target] >= OBJECT_CLASS_MISSILE) {
            ComputeVectorDelta(&g_aShipPosition_00494550[obj],
                               &g_aShipPosition_00494550[target], &delta);
            distance = MaxShort(0,
                (short)(FixedToShortSaturating(Vector_magnitude(&delta)) -
                        g_asObjectCollisionRadius_004950e8[target]));
            if (distance > 1000 ||
                (g_aeObjectClass_00495328[target] == OBJECT_CLASS_MISSILE &&
                 ((g_asMissileProximityFuse_005d38e0[target] * 4 + 4) * 5 * 5 <
                      distance ||
                  g_asShipTactic_00495f30[target] != 4))) {
                damage = 0;
            } else {
                if (distance > 750)
                    distance = 40;
                else if (distance > 500)
                    distance = 30;
                else
                    distance = find_ratio(0, 500, distance, 8, 25);
                damage = (short)((blastDamage / distance) / distance);
                if (obj != -1 &&
                    g_aeObjectClass_00495328[target] >=
                        OBJECT_CLASS_CAPITAL_SHIP &&
                    g_aeObjectClass_00495328[obj] == OBJECT_CLASS_MISSILE &&
                    g_asObjectType_00495298[obj] != WC2_OBJECT_TYPE_TORPEDO)
                    damage = 0;
                if (target == 0)
                    damage = MaxShort(1, (short)(damage * 3 >> 2));
            }
            if (damage > 1) {
                NormalizeFixedVector(&delta);
                ScaleFixedVector(&delta, (int)damage << 8, &force);
                apply_force_to_objects_center(&force, target);
                if (g_asObjectType_00495298[obj] != WC2_OBJECT_TYPE_TORPEDO) {
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

/* Function start: 0x414835 */
short explode(short attacker, short victim)
{
    short result;
    short object;

    if (victim < 10)
        g_anShipCloakState_00496020[victim] = 2;
    if (g_bExplosionTraversalIdle_00492e18 != 0) {
        for (object = 0; object < WC2_SPACE_OBJECT_COUNT; object++)
            g_abExplosionObjectVisited_005d3870[object] = 0;
    }
    g_bExplosionTraversalIdle_00492e18 = 0;
    if (g_abExplosionObjectVisited_005d3870[victim] != 0)
        return 1;
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
        }
        if (RandomBelowOrEqual(1) == 0)
            return 0;
    }
    if (g_aeSpecialManeuver_00495600[victim] ==
            SPECIAL_MANEUVER_UNKNOWN_9 &&
        g_aeObjectClass_00495328[victim] >= OBJECT_CLASS_SHIP)
        return 0;
    if (victim == 0) {
        if (g_bPlayerDamageEnabled_0049d77c == 0)
            return 0;
        g_bPlayerDestroyed_005d2fa4 = 1;
        if (g_bDeathSequenceActive_0049da50 == 0)
            g_nArcadeState_0049d75c = 4;
        return 1;
    }
    if (victim == g_nExternalViewShip_00493468)
        g_nExternalViewShip_00493468 = -1;
    if (g_acObjectOwner_00495208[attacker] != -1)
        attacker = g_acObjectOwner_00495208[attacker];
    if (attacker != -1 &&
        g_aeObjectClass_00495328[victim] >= OBJECT_CLASS_SHIP) {
        analyze_kill(attacker, victim);
        for (objective = 0; objective < WC2_MISSION_OBJECTIVE_COUNT;
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
        if (g_aeObjectClass_00495328[victim] == OBJECT_CLASS_SHIP) {
            BeginShipDestructionSequence(victim);
            return 1;
        }
    }
    Explosion(victim);
    return 1;
}

/* Function start: 0x414BE9 */
int send_at_point(short obj, FixedVector *point, short speed)
{
    ComputeVectorDelta(&g_aShipPosition_00494550[obj], point,
                       &g_aShipVelocity_00494898[obj]);
    SetVectorFixedPoint((unsigned int *)&g_aShipVelocity_00494898[obj],
                        speed);
    return 0;
}

/* Function start: 0x414C39 */
short find_child_object(short parent, short objectClass)
{
    short obj;

    for (obj = 0; obj < WC2_SPACE_OBJECT_COUNT; obj++) {
        if ((short)g_acObjectOwner_00495208[obj] == parent &&
            g_aeObjectClass_00495328[obj] == objectClass)
            return obj;
    }
    return -1;
}

/* Function start: 0x410161 */
short IsCapitalShipObject(short obj)
{
    if (g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_CAPITAL_SHIP)
        return 1;
    return 0;
}

/* Function start: 0x410289 */
short GetAdaptiveTurnRate(void)
{
    short turnRate;

    turnRate = g_aasSeriesMissionTurnRate_004930a0[
        g_nCurrentSeries_005c5870][g_nCurrentMission_005c5878];
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
    if (g_nSpaceFrame_00493134 -
            g_nLastAdaptiveDifficultyChangeFrame_00492d60 < 20)
        return;
    g_nAdaptiveDifficulty_005d3844--;
    if (g_nAdaptiveDifficulty_005d3844 < 0)
        g_nAdaptiveDifficulty_005d3844 = 0;
    g_nLastAdaptiveDifficultyChangeFrame_00492d60 =
        g_nSpaceFrame_00493134;
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
    return 0;
}

/* Function start: 0x414DC0 */
void fire(short obj, short target)
{
    ObjectTypeData *weaponData;
    FixedVector direction;
    short weaponVelocity;
    short weaponType;
    short firedObject;
    short range;
    short weaponRange;
    short targetInRange;
    short weapon;
    short shouldFire;
    short canFire;
    short fireMissile;
    short other;
    short nearestRange;

    canFire = g_asObjectCounter_00494be0[obj] <= 0;
    if (g_asShipFriendlyFireCooldown_00496090[obj] > 0)
        return;
    if (CountShipProjectiles(obj) > g_nAdaptiveDifficulty_005d3844)
        return;

    range = g_nTargetRange_0049319c;
    if (RandomBelowOrEqual(0x13) == 0 &&
        RandomBelowOrEqual(7000) > range)
        fireMissile = 1;
    else
        fireMissile = 0;
    if (fireMissile != 0 &&
        g_aeObjectClass_00495328[target] >= OBJECT_CLASS_CAPITAL_SHIP &&
        g_nYourWingman_0049346c == obj &&
        (unsigned short)RandomInRange(0, 100) > 10)
        fireMissile = 0;
    if (fireMissile != 0 &&
        find_child_ship(obj, OBJECT_CLASS_MISSILE, target) != -1)
        fireMissile = 0;
    if (g_bAiMissileFiringEnabled_00492d58 == 0)
        fireMissile = 0;

    weapon = 0;
    for (; weapon < (signed char)g_aShipWeapons_004956b0[obj][0];
         weapon++) {
        weaponType = ((ShipWeaponSlot *)&g_aShipWeapons_004956b0[obj][1])[
                         weapon].weaponType;
        weaponData = &g_aObjectTypeData_00496d30[weaponType];
        weaponType = ((ShipWeaponSlot *)&g_aShipWeapons_004956b0[obj][1])[
                         weapon].type;
        weaponVelocity = weaponData->maximumVelocity;
        weaponRange =
            (short)(*(int *)&weaponData->lifetime * weaponVelocity);
        targetInRange = weaponRange > range;
        shouldFire = 0;

        switch (weaponData->objectClass) {
        case OBJECT_CLASS_PROJECTILE:
            if (CanShipWeaponDamageTarget(obj, target) != 0 &&
                canFire != 0 && targetInRange != 0) {
                switch (weaponType) {
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
                    weapon].disabled = 0;
            else
                ((ShipWeaponSlot *)&g_aShipWeapons_004956b0[obj][1])[
                    weapon].disabled = 1;
            if (shouldFire != 0) {
                firedObject = fire_weapon(obj, weapon);
                if (g_anShipCloakState_00496020[obj] == 1)
                    ResetShipCloakState(obj);
                if (weaponType == 12) {
                    ComputeVectorDelta(
                        &g_aShipPosition_00494550[firedObject],
                        &g_aShipPosition_00494550[target], &direction);
                    launch_object(obj, firedObject, direction,
                                  real_velocity(firedObject));
                }
            }
            break;

        case OBJECT_CLASS_MISSILE:
            if (fireMissile != 0 && targetInRange != 0) {
                switch (weaponType) {
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
                fire_weapon(obj, weapon);
                fireMissile = 0;
            }
            break;
        }
    }
}

/* Function start: 0x415508 */
short hemisphere(FixedVector *target, FixedVector *parent,
                 FixedVector *hardpoint)
{
    FixedVector parentFromHardpoint;
    FixedVector parentFromTarget;

    ComputeVectorDelta(hardpoint, parent, &parentFromHardpoint);
    ComputeVectorDelta(target, parent, &parentFromTarget);
    return vector_angle(parentFromTarget, parentFromHardpoint);
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
    return -v;
}

/* Function start: 0x41565E */
int RandomFixedAimComponent(short radius, short speed, short maximum)
{
    short aim;

    aim = RandomSign(MinShort(
        maximum, (short)(RandomBelowOrEqual(radius) + speed)));
    return aim << 8;
}

/* Function start: 0x4156A7 */
short SpawnFlakBurst(short obj, short range, FixedVector *hardpoint,
                     short burstMode)
{
    FixedVector randomAim;
    FixedVector aimPoint;
    FixedVector targetMotion;
    short target;
    short explosion;
    short aimRadius;
    short targetSpeed;
    short chance;
    short predictionSteps;

    target = g_acShipTarget_00495f20[obj];
    explosion = new_object(0x24, obj);
    if (explosion != -1) {
        aimRadius = MaxShort(400, (short)(range >> 3));
        targetSpeed = (short)(real_velocity(target) << 3);
        zero_vector(&randomAim);
        if (burstMode == 0) {
            randomAim.x =
                RandomFixedAimComponent(aimRadius, targetSpeed, 1000);
            randomAim.y =
                RandomFixedAimComponent(aimRadius, targetSpeed, 1000);
            randomAim.z =
                RandomFixedAimComponent(aimRadius, targetSpeed, 1000);
        }
        AddFixedVectors(&g_aShipPosition_00494550[target], &randomAim,
                        &aimPoint);
        chance = RandomBelowOrEqual(100);
        if (burstMode != 0) {
            if (g_asObjectCounter_00494be0[obj] <= 0) {
                g_aShipPosition_00494550[explosion] = *hardpoint;
                predictionSteps = (short)(range / 70);
                if (predictionSteps < 0)
                    predictionSteps = 0;
                if (predictionSteps > 20)
                    predictionSteps = 20;
                ScaleFixedVector(&g_aShipVelocity_00494898[target],
                                 predictionSteps << 8, &targetMotion);
                AddFixedVectors(&targetMotion, &aimPoint, &aimPoint);
                fire_flack(obj, explosion, range, &aimPoint, burstMode);
                g_asObjectCounter_00494be0[obj] =
                    (short)(RandomBelow(10) + 7);
            }
        } else if ((g_asObjectCounter_00494be0[obj] <= 0 && chance < 40) ||
                   chance < 8) {
            g_aShipPosition_00494550[explosion] = *hardpoint;
            fire_flack(obj, explosion, range, &aimPoint, burstMode);
            g_asObjectCounter_00494be0[obj] =
                (short)(RandomBelow(10) + 7);
        } else if (g_asObjectType_00495298[
                       (signed char)g_acShipTarget_00495f20[obj]] != 0x13) {
            g_aShipPosition_00494550[explosion] = aimPoint;
            explosion_shock_wave(
                explosion,
                g_aObjectTypeData_00496d30[0x0c].explosionDamage);
        }
    }
    return explosion;
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
         weapon++, slot++) {
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

/* Function start: 0x415E2C */
short fire_weapon(short obj, short weapon)
{
    ObjectTypeData *weaponData;
    FixedVector vector;
    FixedVector launchOffset;
    int interceptionScale;
    short proximityDistance;
    short leadDistance;
    short weaponClass;
    short weaponType;
    short projectile;
    short weaponDataIndex;
    short other;
    short projectileSpeed;
    short targetRange;

    projectileSpeed = 10;
    weaponType = ((ShipWeaponSlot *)&g_aShipWeapons_004956b0[obj][1])[
                     weapon].type;
    weaponDataIndex =
        ((ShipWeaponSlot *)&g_aShipWeapons_004956b0[obj][1])[
            weapon].weaponType;
    weaponClass = g_aObjectTypeData_00496d30[weaponType].objectClass;
    if (weaponType == 0x0c) {
        weaponType = 7;
        weaponClass = 8;
    }
    if (weaponClass == OBJECT_CLASS_MISSILE) {
        if (get_ship_slot() == -1) {
            for (other = 1; other < 10; other++) {
                if (g_aeObjectClass_00495328[other] ==
                        OBJECT_CLASS_MISSILE &&
                    g_asObjectType_00495298[other] == 0x14 &&
                    g_acObjectOwner_00495208[other] == obj)
                    explode(other, other);
            }
        }
        projectile = initialize_ship(weaponType, obj, 0);
    } else {
        projectile = new_object(weaponType, obj);
    }
    if (projectile != -1) {
        weaponData = &g_aObjectTypeData_00496d30[weaponDataIndex];
        copy_frame(obj, projectile);
        if (weaponClass == OBJECT_CLASS_PROJECTILE) {
            g_asObjectDamage_00495178[projectile] =
                weaponData->damageCapacity;
            projectileSpeed = g_aObjectTypeData_00496d30[
                g_acObjectType_00493980[projectile]].maximumVelocity;
            g_asShipWeaponEnergy_00495590[obj] =
                (short)(g_asShipWeaponEnergy_00495590[obj] -
                        weaponData->animationDelay);
        }
        child_object(weapon, projectile, obj);
        g_asObjectCounter_00494be0[projectile] =
            *(int *)&g_aObjectTypeData_00496d30[
                weaponDataIndex].lifetime;
        vector_component_in_dir(&g_aShipVelocity_00494898[obj],
                                &g_aShipForwardVector_00494208[projectile],
                                &g_aShipVelocity_00494898[projectile]);
        if (weaponClass == OBJECT_CLASS_PROJECTILE) {
            if (obj == 0 && g_bTargetLockActive_0049ae80 != 0 &&
                g_bMissileLockAcquired_0049b2b0 != 0) {
                targetRange = distance_from_object(
                    0, g_acShipTarget_00495f20[0]);
                leadDistance = (short)(targetRange + 100);
            } else {
                leadDistance = (short)((*(int *)&weaponData->lifetime + 5) *
                                       weaponData->maximumVelocity);
            }
            ScaleFixedVector(&g_aShipForwardVector_00494208[obj],
                             leadDistance << 8, &vector);
            AddFixedVectors(&g_aShipPosition_00494550[obj], &vector,
                            &vector);
            point_at(projectile, vector);
        }
        ScaleFixedVector(&g_aShipForwardVector_00494208[projectile],
                         (int)projectileSpeed << 8, &vector);
        AddFixedVectors(&vector, &g_aShipVelocity_00494898[projectile],
                        &g_aShipVelocity_00494898[projectile]);
        RecordCannedSceneObjectEvent(projectile, 0);
        if (weaponClass == OBJECT_CLASS_MISSILE) {
            ScaleFixedVector(&g_aShipUpVector_00493ec0[obj], 0xa00,
                             &vector);
            AddFixedVectors(&vector, &g_aShipVelocity_00494898[projectile],
                            &g_aShipVelocity_00494898[projectile]);
            if (obj == 0 && weaponType != 0x14)
                RemovePlayerReleaseWeapon(weapon);
            else
                remove_weapon(obj, weapon);
            g_aeSpecialManeuver_00495600[projectile] = -1;
            g_asShipManeuver_00495f48[projectile] =
                g_aeSpecialManeuver_00495600[projectile];
            g_asShipTactic_00495f30[projectile] = 1;
            g_asPilotLevel_00495d60[projectile] = -1;
            g_abShipEjectionSequenceEnabled_00496120[projectile] = 0;
            g_asObjectCounter_00494be0[projectile] = 5;
            g_acObjectCollisionGraceTicks_00494d48[projectile] = 20;
            switch (weaponType) {
            case 0x0f:
                steady_object(projectile);
                g_acShipTarget_00495f20[projectile] =
                    g_acShipTarget_00495f20[obj];
                g_asObjectCounter_00494be0[projectile] = 1;
                g_anShipSpeed_00494e20[projectile] =
                    get_ship_max_velocity(projectile) << 8;
                if (g_acShipTarget_00495f20[projectile] != -1) {
                    ComputeVectorDelta(&g_aShipPosition_00494550[obj],
                        &g_aShipPosition_00494550[
                            g_acShipTarget_00495f20[projectile]], &vector);
                    interceptionScale =
                        Vector_magnitude(&vector) /
                        get_ship_max_velocity(projectile);
                    ScaleFixedVector(&g_aShipVelocity_00494898[
                        g_acShipTarget_00495f20[projectile]],
                        interceptionScale, &vector);
                    AddFixedVectors(&g_aShipPosition_00494550[
                        g_acShipTarget_00495f20[projectile]], &vector,
                        &vector);
                    point_at(projectile, vector);
                }
                break;
            case 0x10:
            case 0x12:
                g_acShipTarget_00495f20[projectile] =
                    g_acShipTarget_00495f20[obj];
                break;
            case 0x13:
                g_acShipTarget_00495f20[projectile] =
                    g_acShipTarget_00495f20[obj];
                if (g_nCurrentView_00492fa8 == 4)
                    g_acShipTarget_00495f20[projectile] =
                        (signed char)g_nSavedPlayerTarget_0049d460;
                proximityDistance = (short)(distance_from_object(
                    obj, g_acShipTarget_00495f20[projectile]) / 1000);
                if (proximityDistance > 9 ||
                    (unsigned short)RandomInRange(0, 15) <
                        proximityDistance)
                    g_asMissileProximityFuse_005d38e0[projectile] = 1;
                else
                    g_asMissileProximityFuse_005d38e0[projectile] = 0;
                break;
            case 0x11:
                g_acShipTarget_00495f20[projectile] = -1;
                g_asObjectCounter_00494be0[projectile] = 15;
                break;
            case 0x14:
                g_asObjectCounter_00494be0[projectile] = 120;
                g_acShipTarget_00495f20[projectile] = -1;
                ScaleFixedVector(&g_aShipForwardVector_00494208[obj],
                                 0xa00, &launchOffset);
                SubtractFixedVectors(&g_aShipVelocity_00494898[obj],
                                     &launchOffset,
                                     &g_aShipVelocity_00494898[projectile]);
                break;
            }
        }
        if (obj == 0) {
            if (weaponClass == OBJECT_CLASS_PROJECTILE) {
                g_asObjectCounter_00494be0[obj] =
                    g_acGunRefireDelay_00492e1c[
                        weaponType - 7];
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
        switch (weaponType) {
        case 7:
        case 8:
        case 0x0d:
            PlaySfxWaveFileByNumber(8, projectile, 0);
            break;
        case 9:
        case 0x0c:
        case 0x14:
            PlaySfxWaveFileByNumber(5, projectile, 0);
            break;
        case 0x0a:
            PlaySfxWaveFileByNumber(0x27, projectile, 0);
            break;
        case 0x0f:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
            PlaySfxWaveFileByNumber(1, projectile, 0);
            break;
        }
    }
    return projectile;
}

/* Function start: 0x446710 */
void BeginShipDestructionSequence(short obj)
{
    send_message(obj, 9);
    g_nPendingEjectionShip_005d1bc4 = obj;
    set_special(obj, SPECIAL_MANEUVER_UNKNOWN_9);
    if (g_aObjectTypeData_00496d30[WC2_OBJECT_TYPE(OBJECT_TYPE_EXPLOSION2)].shapeSet != 0)
        g_asObjectCounter_00494be0[obj] = 8;
    else
        g_asObjectCounter_00494be0[obj] = 4;
    if (g_aObjectTypeData_00496d30[WC2_OBJECT_TYPE(OBJECT_TYPE_EXPLOSION2)].shapeSet != 0)
        g_asShipExplosionStageTimer_005d3850[obj] = 8;
    else
        g_asShipExplosionStageTimer_005d3850[obj] = 4;
    g_aMissionShips_00492290[g_asShipMissionIndex_00495d00[obj]].state = 3;
    ShipExplosion(obj);
    explosion_shock_wave(
        obj,
        g_aObjectTypeData_00496d30[
            (signed char)g_acObjectType_00493980[obj]].explosionDamage);
    if (g_asObjectScreenX_00493598[obj] != -32767)
        PlaySfxWaveFileByNumber(4, obj, 0);
    g_bMissionDeathSequencePending_0049b720 = 1;
    g_bMissionEjectionPodSpawned_0049b724 = 0;
}

/* Function start: 0x446823 */
void SpawnMissionEjectionPod(short obj)
{
    short initialShip;

    if (g_aObjectTypeData_00496d30[
            WC2_OBJECT_TYPE_EJECTION_POD].shapeSet == 0) {
        g_aObjectTypeData_00496d30[
            WC2_OBJECT_TYPE_EJECTION_POD].shapeSet =
            FetchDiskPacketRetrying("pilotanm.vga", 2, 0);
    }
    set_objects_data(obj, WC2_OBJECT_TYPE_EJECTION_POD, -1, 0);
    zero_vector(&g_aShipVelocity_00494898[obj]);
    RecordCannedSceneObjectEvent(obj, 0);
    g_bMissionEjectionPodSpawned_0049b724 = 1;
    for (initialShip = 0; initialShip < 8; initialShip++) {
        if (g_stMissionHeader_005d3e70.initialMissionShips[initialShip] ==
                g_asShipMissionIndex_00495d00[obj]) {
            g_acMissionShipStatusFlags_005d2fb0[initialShip] |= 2;
            if (obj == g_nYourWingman_0049346c)
                g_nYourWingman_0049346c = -1;
        }
    }
}
