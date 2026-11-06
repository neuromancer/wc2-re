/*
 *  Fleet overview view.
 *
 *  Retail object 0x44ADE0-0x44B1FF (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x44ADE0 */
void SetFleetOverviewView(short initializeCockpit)
{
    FixedVector centre;
    FixedVector axes;
    FixedVector viewOffset;
    short shipCount;
    int range;
    int playerRange;
    int cameraDistance;
    int furthestRange;
    short shipSlot;

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
        zero_vector(&g_aShipVelocity[EYE_OBJECT]);
    }

    for (shipSlot = 0; shipSlot < 10; shipSlot++) {
        if (g_aeObjectClass[shipSlot] >= OBJECT_CLASS_SHIP)
            shipCount++;
    }

    if (shipCount > 1) {
        zero_vector(&centre);
        for (shipSlot = 0; shipSlot < 10; shipSlot++) {
            if (g_aeObjectClass[shipSlot] >= OBJECT_CLASS_SHIP) {
                centre.x += g_aShipPosition[shipSlot].x / shipCount;
                centre.y += g_aShipPosition[shipSlot].y / shipCount;
                centre.z += g_aShipPosition[shipSlot].z / shipCount;
            }
        }

        furthestRange = 0x4b000;
        for (shipSlot = 9; shipSlot >= 0; shipSlot--) {
            if (g_aeObjectClass[shipSlot] >= OBJECT_CLASS_SHIP) {
                ComputeVectorDelta(&centre,
                                   &g_aShipPosition[shipSlot],
                                   &viewOffset);
                range = Vector_magnitude(&viewOffset);
                if (furthestRange < range)
                    furthestRange = range;
                if (shipSlot == 0)
                    playerRange = range;
            }
        }
        if (furthestRange > 0x1f4000)
            cameraDistance = playerRange;
        else
            cameraDistance = furthestRange;
    } else {
        cameraDistance = furthestRange = playerRange = 0x4b000;
        position_relative_ijk(&centre, 0, 400, 400, 400);
    }

    cameraDistance = (cameraDistance >> 3) * 9 + 0x2bc00;
    g_aShipPosition[SCRATCH_VIEW_OBJECT] = centre;
    g_aShipRightVector[SCRATCH_VIEW_OBJECT] = axes;
    g_aShipUpVector[SCRATCH_VIEW_OBJECT] = axes;
    point_at(SCRATCH_VIEW_OBJECT, g_aShipPosition[0]);

    ScaleFixedVector(
        &g_aShipRightVector[SCRATCH_VIEW_OBJECT],
        cameraDistance >> 2, &viewOffset);
    AddFixedVectors(&g_aShipPosition[SCRATCH_VIEW_OBJECT],
                    &viewOffset,
                    &g_aShipPosition[SCRATCH_VIEW_OBJECT]);
    ScaleFixedVector(&g_aShipUpVector[SCRATCH_VIEW_OBJECT],
                     0x9600, &viewOffset);
    AddFixedVectors(&g_aShipPosition[SCRATCH_VIEW_OBJECT],
                    &viewOffset,
                    &g_aShipPosition[SCRATCH_VIEW_OBJECT]);
    ScaleFixedVector(
        &g_aShipForwardVector[SCRATCH_VIEW_OBJECT],
        cameraDistance, &viewOffset);
    AddFixedVectors(&g_aShipPosition[SCRATCH_VIEW_OBJECT],
                    &viewOffset,
                    &g_aShipPosition[SCRATCH_VIEW_OBJECT]);

    g_aShipPosition[EYE_OBJECT] =
        g_aShipPosition[SCRATCH_VIEW_OBJECT];
    g_aShipRightVector[EYE_OBJECT] = axes;
    g_aShipUpVector[EYE_OBJECT] = axes;
    if (furthestRange < 0x271000)
        point_at(EYE_OBJECT, centre);
    else
        point_at(EYE_OBJECT, g_aShipPosition[0]);
}
