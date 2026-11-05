/*
 *  Fleet overview view.
 *
 *  Retail object 0x44ADE0-0x44B1FF (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x44ADE0 */
void SetFleetOverviewView(int initializeCockpit)
{
    FixedVector centre;
    FixedVector axes;
    FixedVector viewOffset;
    int shipCount;
    int furthestRange;
    int playerRange;
    int cameraDistance;
    int range;
    int shipSlot;

    axes.x = 0xff;
    axes.y = 0xff;
    axes.z = 0xff;
    shipCount = 0;
    if (initializeCockpit != 0) {
        free_view_buffer();
        if ((int)GetLargestFreeMemoryBlockByType(0) < 66000)
            initialize_cockpit(4);
        else
            initialize_cockpit(6);
        zero_vector(&g_aShipVelocity_00494898[EYE_OBJECT]);
    }

    shipSlot = 0;
    do {
        if (g_aeObjectClass_00495328[shipSlot] >= OBJECT_CLASS_SHIP)
            shipCount++;
        shipSlot++;
    } while (shipSlot < 10);

    if (shipCount > 1) {
        zero_vector(&centre);
        shipSlot = 0;
        do {
            if (g_aeObjectClass_00495328[shipSlot] >= OBJECT_CLASS_SHIP) {
                centre.x += g_aShipPosition_00494550[shipSlot].x / shipCount;
                centre.y += g_aShipPosition_00494550[shipSlot].y / shipCount;
                centre.z += g_aShipPosition_00494550[shipSlot].z / shipCount;
            }
            shipSlot++;
        } while (shipSlot < 10);

        furthestRange = 0x4b000;
        playerRange = 0xff;
        shipSlot = 9;
        do {
            if (g_aeObjectClass_00495328[shipSlot] >= OBJECT_CLASS_SHIP) {
                ComputeVectorDelta(&centre,
                                   &g_aShipPosition_00494550[shipSlot],
                                   &viewOffset);
                range = Vector_magnitude(&viewOffset);
                if (furthestRange < range)
                    furthestRange = range;
                if (shipSlot == 0)
                    playerRange = range;
            }
            shipSlot--;
        } while (shipSlot >= 0);
        if (furthestRange <= 0x1f4000)
            playerRange = furthestRange;
    } else {
        furthestRange = 0x4b000;
        playerRange = 0x4b000;
        position_relative_ijk(&centre, 0, 400, 400, 400);
    }

    cameraDistance = (playerRange >> 3) * 9 + 0x2bc00;
    g_aShipPosition_00494550[SCRATCH_VIEW_OBJECT] = centre;
    g_aShipRightVector_00493b78[SCRATCH_VIEW_OBJECT] = axes;
    g_aShipUpVector_00493ec0[SCRATCH_VIEW_OBJECT] = axes;
    point_at(SCRATCH_VIEW_OBJECT, g_aShipPosition_00494550[0]);

    ScaleFixedVector(
        &g_aShipRightVector_00493b78[SCRATCH_VIEW_OBJECT],
        cameraDistance >> 2, &viewOffset);
    AddFixedVectors(&g_aShipPosition_00494550[SCRATCH_VIEW_OBJECT],
                    &viewOffset,
                    &g_aShipPosition_00494550[SCRATCH_VIEW_OBJECT]);
    ScaleFixedVector(&g_aShipUpVector_00493ec0[SCRATCH_VIEW_OBJECT],
                     0x9600, &viewOffset);
    AddFixedVectors(&g_aShipPosition_00494550[SCRATCH_VIEW_OBJECT],
                    &viewOffset,
                    &g_aShipPosition_00494550[SCRATCH_VIEW_OBJECT]);
    ScaleFixedVector(
        &g_aShipForwardVector_00494208[SCRATCH_VIEW_OBJECT],
        cameraDistance, &viewOffset);
    AddFixedVectors(&g_aShipPosition_00494550[SCRATCH_VIEW_OBJECT],
                    &viewOffset,
                    &g_aShipPosition_00494550[SCRATCH_VIEW_OBJECT]);

    g_aShipPosition_00494550[EYE_OBJECT] =
        g_aShipPosition_00494550[SCRATCH_VIEW_OBJECT];
    g_aShipRightVector_00493b78[EYE_OBJECT] = axes;
    g_aShipUpVector_00493ec0[EYE_OBJECT] = axes;
    if (furthestRange < 0x271000)
        point_at(EYE_OBJECT, centre);
    else
        point_at(EYE_OBJECT, g_aShipPosition_00494550[0]);
}
