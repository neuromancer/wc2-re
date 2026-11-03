/*
 *  Available main memory.
 *
 *  Retail object 0x469AD0-0x469BDF (link order; see docs/ORDER.md).
 */
#include "game.h"
#ifndef SDL_PORT
#include <malloc.h>
#endif

/* Function start: 0x469AD0 */
/* Reports a fixed 8 MB; the DOS near-heap free-space sum follows the early
 * return. */
unsigned int GetAvailableMainMemory(void)
{
    NearHeapBlock *descriptor;
    int descriptorAddress;
    int totalFree;

    return 0x7c0600;
    totalFree = 0;
    descriptorAddress =
        g_nNearArenaSize_005d3050 + g_nNearHeapBase_005d3054 - 8;
    while (descriptorAddress >= g_nNearHeapFirstDescriptor_005d3058) {
        descriptor = DosNearPtrToFar(descriptorAddress);
        if ((descriptor->sizeAndFlags & 0x80000000) == 0)
            totalFree += (int)(descriptor->sizeAndFlags & 0xfffff);
        descriptorAddress -= 8;
    }
    return totalFree;
}

/* Function start: 0x469B49 */
/* Reports a fixed 8 MB; the DOS near-heap largest-block search follows the
 * early return. */
unsigned int GetLargestMainMemoryBlock(void)
{
    NearHeapBlock *descriptor;
    int descriptorAddress;
    int largest;

    return 0x7c0600;
    largest = 0;
    descriptorAddress =
        g_nNearArenaSize_005d3050 + g_nNearHeapBase_005d3054 - 8;
    while (descriptorAddress >= g_nNearHeapFirstDescriptor_005d3058) {
        descriptor = DosNearPtrToFar(descriptorAddress);
        if ((descriptor->sizeAndFlags & 0x80000000) == 0 &&
            (int)(descriptor->sizeAndFlags & 0xfffff) > largest)
            largest = descriptor->sizeAndFlags & 0xfffff;
        descriptorAddress -= 8;
    }
    return largest;
}
