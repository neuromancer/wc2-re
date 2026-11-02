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
    FixedVector orientation;
    FixedVector offset;
    int shipCount;
    int maximumRange;
    int playerRange;
    int cameraDistance;
    int range;
    int object;

    orientation.x = 0xff;
    orientation.y = 0xff;
    orientation.z = 0xff;
    shipCount = 0;
    if (initializeCockpit != 0) {
        free_view_buffer();
        if ((int)GetLargestFreeMemoryBlockByType(0) < 66000)
            initialize_cockpit(4);
        else
            initialize_cockpit(6);
        zero_vector(&g_aShipVelocity_00494898[EYE_OBJECT]);
    }

    object = 0;
    do {
        if (g_aeObjectClass_00495328[object] >= OBJECT_CLASS_SHIP)
            shipCount++;
        object++;
    } while (object < 10);

    if (shipCount > 1) {
        zero_vector(&centre);
        object = 0;
        do {
            if (g_aeObjectClass_00495328[object] >= OBJECT_CLASS_SHIP) {
                centre.x += g_aShipPosition_00494550[object].x / shipCount;
                centre.y += g_aShipPosition_00494550[object].y / shipCount;
                centre.z += g_aShipPosition_00494550[object].z / shipCount;
            }
            object++;
        } while (object < 10);

        maximumRange = 0x4b000;
        playerRange = 0xff;
        object = 9;
        do {
            if (g_aeObjectClass_00495328[object] >= OBJECT_CLASS_SHIP) {
                ComputeVectorDelta(&centre,
                                   &g_aShipPosition_00494550[object],
                                   &offset);
                range = Vector_magnitude(&offset);
                if (maximumRange < range)
                    maximumRange = range;
                if (object == 0)
                    playerRange = range;
            }
            object--;
        } while (object >= 0);
        if (maximumRange <= 0x1f4000)
            playerRange = maximumRange;
    } else {
        maximumRange = 0x4b000;
        playerRange = 0x4b000;
        position_relative_ijk(&centre, 0, 400, 400, 400);
    }

    cameraDistance = (playerRange >> 3) * 9 + 0x2bc00;
    g_aShipPosition_00494550[SCRATCH_VIEW_OBJECT] = centre;
    g_aShipRightVector_00493b78[SCRATCH_VIEW_OBJECT] = orientation;
    g_aShipUpVector_00493ec0[SCRATCH_VIEW_OBJECT] = orientation;
    point_at(SCRATCH_VIEW_OBJECT, g_aShipPosition_00494550[0]);

    ScaleFixedVector(
        &g_aShipRightVector_00493b78[SCRATCH_VIEW_OBJECT],
        cameraDistance >> 2, &offset);
    AddFixedVectors(&g_aShipPosition_00494550[SCRATCH_VIEW_OBJECT],
                    &offset,
                    &g_aShipPosition_00494550[SCRATCH_VIEW_OBJECT]);
    ScaleFixedVector(&g_aShipUpVector_00493ec0[SCRATCH_VIEW_OBJECT],
                     0x9600, &offset);
    AddFixedVectors(&g_aShipPosition_00494550[SCRATCH_VIEW_OBJECT],
                    &offset,
                    &g_aShipPosition_00494550[SCRATCH_VIEW_OBJECT]);
    ScaleFixedVector(
        &g_aShipForwardVector_00494208[SCRATCH_VIEW_OBJECT],
        cameraDistance, &offset);
    AddFixedVectors(&g_aShipPosition_00494550[SCRATCH_VIEW_OBJECT],
                    &offset,
                    &g_aShipPosition_00494550[SCRATCH_VIEW_OBJECT]);

    g_aShipPosition_00494550[EYE_OBJECT] =
        g_aShipPosition_00494550[SCRATCH_VIEW_OBJECT];
    g_aShipRightVector_00493b78[EYE_OBJECT] = orientation;
    g_aShipUpVector_00493ec0[EYE_OBJECT] = orientation;
    if (maximumRange < 0x271000)
        point_at(EYE_OBJECT, centre);
    else
        point_at(EYE_OBJECT, g_aShipPosition_00494550[0]);
}
