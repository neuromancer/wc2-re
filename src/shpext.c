/*
 *  Shape frame extent.
 *
 *  Retail object 0x447170-0x4471AF (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x447170 */
short GetShapeFrameExtent(short x, short y,
                                    unsigned char *shape, short frame,
                                    short extent)
{
    short bounds[4];

    GetShapeFrameBounds(bounds, x, y, shape, frame);
    return bounds[extent];
}
