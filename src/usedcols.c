/*
 *  Collects the palette indices in use.
 *
 *  Retail object 0x4589D0-0x458A8F (link order; see docs/ORDER.md).
 */
#include "game.h"
#ifndef SDL_PORT
#include <malloc.h>
#endif

#pragma function(memset)
/* Function start: 0x4589D0 */
short CollectActivePaletteIndices(Viewport *viewport,
                                  unsigned char *indices,
                                  short capacity)
{
    short count;
    short index;
    unsigned char *map;

    count = 0;
    map = AllocateTaggedMemory((unsigned int)capacity, 0);
    if (map == 0)
        return 0;

    memset(map, 0, (unsigned int)capacity);
    EstablishColors(viewport, map);
    for (index = 0; index < capacity; index++) {
        if (map[index] != 0) {
            indices[count] = (unsigned char)index;
            count++;
        }
    }
    ReleasePacketHandle(map);
    return count;
}
