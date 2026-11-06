/*
 *  Weapon selection.
 *
 *  Retail object 0x461480-0x4618DF (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x461480 */
short find_next_gun(short obj, short currentGun)
{
    unsigned char *loadoutTable;
    short earliest;
    short foundCurrent;
    short count;
    ShipWeaponSlot *weaponSlot;
    short i;

    loadoutTable = g_aShipWeapons[obj];
    foundCurrent = 0;
    earliest = -1;
    count = (signed char)loadoutTable[0];
    weaponSlot = (ShipWeaponSlot *)&loadoutTable[1];
    for (i = 0; i < count; i++, weaponSlot++) {
        if (g_aObjectTypeData[weaponSlot->type].objectClass ==
                OBJECT_CLASS_PROJECTILE) {
            if (weaponSlot->type != OBJECT_DATA_TURRET_GUN) {
                if (earliest == -1)
                    earliest = weaponSlot->type;
                if (foundCurrent == 0) {
                    if (weaponSlot->type == currentGun)
                        foundCurrent = 1;
                } else if (weaponSlot->type != currentGun) {
                    return weaponSlot->type;
                }
            }
        }
    }
    if (foundCurrent != 0 && earliest != currentGun)
        return 0x80;
    return earliest;
}

/* Function start: 0x4615A2 */
int select_guns(short selectedGun)
{
    unsigned char *weapons;
    short any;
    short weaponCount;
    short i;
    ShipWeaponSlot *weaponSlot;

    weapons = g_aShipWeapons[0];
    any = 0;
    weaponCount = (signed char)weapons[0];
    i = 0;
    weaponSlot = (ShipWeaponSlot *)&weapons[1];
    for (; i < weaponCount; i++, weaponSlot++) {
        if (g_aObjectTypeData[weaponSlot->type].objectClass ==
                OBJECT_CLASS_PROJECTILE) {
            if (weaponSlot->type == selectedGun || selectedGun == 0x80) {
                weaponSlot->disabled = 0;
                any = 1;
            } else {
                weaponSlot->disabled = 1;
            }
        }
    }
    if (any != 0)
        return selectedGun;
    return -1;
}

/* Function start: 0x46166D */
void select_new_gun(void)
{
    g_nSelectedGunType =
        select_guns(find_next_gun(0, g_nSelectedGunType));
    if (get_mode(0) == 1)
        InvalidateVduMode(0);
}

/* Function start: 0x4616B8 */
void select_new_release_weapon(short preferredType)
{
    signed char weaponCount;
    short firstFound;
    signed char index;
    short weaponType;

    weaponCount = g_aShipWeapons[0][0];
    index = (signed char)g_nSelectedReleaseWeaponIndex;
    index++;
    if (weaponCount <= index)
        index = 0;
    if (g_nSelectedReleaseWeaponIndex == -1) {
        if (preferredType != -1) {
            for (index = 0; weaponCount > index; index++) {
                if (((ShipWeaponSlot *)&g_aShipWeapons[0][1])[
                        index].type == preferredType) {
                    ((ShipWeaponSlot *)&g_aShipWeapons[0][1])[
                        index].disabled = 0;
                    g_nSelectedReleaseWeaponIndex = index;
                    break;
                }
            }
        }
        if (g_nSelectedReleaseWeaponIndex == -1) {
            for (index = 0; weaponCount > index; index++) {
                weaponType =
                    ((ShipWeaponSlot *)&g_aShipWeapons[0][1])[
                        index].weaponType;
                if (g_aObjectTypeData[weaponType].objectClass !=
                    OBJECT_CLASS_PROJECTILE) {
                    ((ShipWeaponSlot *)&g_aShipWeapons[0][1])[
                        index].disabled = 0;
                    g_nSelectedReleaseWeaponIndex = index;
                    break;
                }
            }
        }
    } else {
        firstFound = index;
        while (index != g_nSelectedReleaseWeaponIndex) {
            weaponType =
                ((ShipWeaponSlot *)&g_aShipWeapons[0][1])[
                    index].weaponType;
            if (g_aObjectTypeData[weaponType].objectClass !=
                    OBJECT_CLASS_PROJECTILE &&
                ((ShipWeaponSlot *)&g_aShipWeapons[0][1])[
                    index].type !=
                    ((ShipWeaponSlot *)&g_aShipWeapons[0][1])[
                        g_nSelectedReleaseWeaponIndex].type) {
                ((ShipWeaponSlot *)&g_aShipWeapons[0][1])[
                    g_nSelectedReleaseWeaponIndex].disabled = 1;
                ((ShipWeaponSlot *)&g_aShipWeapons[0][1])[
                    index].disabled = 0;
                g_nSelectedReleaseWeaponIndex = index;
                break;
            }
            index++;
            if (weaponCount <= index)
                index = 0;
            if (index == firstFound)
                break;
        }
    }
    if (get_mode(0) == 1)
        InvalidateVduMode(0);
}
