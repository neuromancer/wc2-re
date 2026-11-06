/*
 *  Near heap allocator.
 *
 *  Retail object 0x4207B0-0x42120F (link order; see docs/ORDER.md).
 */
#define NEARHEAP_C
#include "game.h"

volatile short g_nNearHeapActive_00493044 = 0;
volatile short g_nNearHeapMaxDescriptors_00493048 = 0x80;
volatile int g_nNearHeapRelocationBytes_0049304c = 0;
#include <float.h>

/* Function start: 0x4207B0 */
short MergeAdjacentNearHeapBlocks(int descriptorAddress)
{
    NearHeapBlock *block;

    block = DosNearPtrToFar(descriptorAddress);
    if ((block->sizeAndFlags & 0x80000000) == 0 &&
        (block[1].sizeAndFlags & 0x80000000) == 0 &&
        block->address + (block->sizeAndFlags & 0xfffff) ==
            block[1].address) {
        block[1].address = block->address;
        block[1].sizeAndFlags += block->sizeAndFlags & 0xfffff;
        while (descriptorAddress > g_nNearHeapFirstDescriptor_005d3058) {
            descriptorAddress -= 8;
            block = DosNearPtrToFar(descriptorAddress);
            block[1].address = block->address;
            block[1].sizeAndFlags = block->sizeAndFlags;
        }
        g_nNearHeapFirstDescriptor_005d3058 += 8;
        return 1;
    }
    return 0;
}

/* Function start: 0x420874 */
int ReleaseNearHeapBlock(int descriptorAddress)
{
    NearHeapBlock *block;

    block = DosNearPtrToFar(descriptorAddress);
    /* The signed read is what keeps the mask out of the descriptor
     * word: the compiler folds the unsigned form into `and [mem]`. */
    block->sizeAndFlags = (int)block->sizeAndFlags & 0x7fffffff;
    if (g_nNearArenaSize_005d3050 + g_nNearHeapBase_005d3054 >
            descriptorAddress + 8 &&
        MergeAdjacentNearHeapBlocks(descriptorAddress) != 0)
        descriptorAddress += 8;
    if (descriptorAddress > g_nNearHeapFirstDescriptor_005d3058)
        MergeAdjacentNearHeapBlocks(descriptorAddress - 8);
    return descriptorAddress;
}

/* Function start: 0x4208FC */
void PurgeNearHeapBlocks(short flags)
{
    NearHeapBlock *entry;
    int descriptorAddress;

    if (g_nNearHeapActive_00493044 != 0) {
        if ((flags & 0x10) != 0) {
            g_nNearHeapFirstDescriptor_005d3058 =
                g_nNearArenaSize_005d3050 + g_nNearHeapBase_005d3054 - 8;
            entry = DosNearPtrToFar(g_nNearHeapFirstDescriptor_005d3058);
            entry->address = g_nNearHeapBase_005d3054;
            entry->sizeAndFlags = g_nNearArenaSize_005d3050 -
                                  g_nNearHeapMaxDescriptors_00493048 * 8;
        } else {
            descriptorAddress =
                g_nNearArenaSize_005d3050 + g_nNearHeapBase_005d3054 - 8;
            while (descriptorAddress >=
                   g_nNearHeapFirstDescriptor_005d3058) {
                entry = DosNearPtrToFar(descriptorAddress);
                if ((entry->sizeAndFlags & 0x40000000) == 0)
                    descriptorAddress =
                        ReleaseNearHeapBlock(descriptorAddress);
                descriptorAddress -= 8;
            }
        }
    }
}

/* Function start: 0x4209C2 */
unsigned short InitializeNearHeap(void)
{
    if (g_nNearHeapActive_00493044 == 0) {
        g_nNearArenaSize_005d3050 = (short)GetNavRangeSentinel();
        g_pNearHeapAllocationPtr_005d305c = 0;
        if (g_nNearHeapMaxDescriptors_00493048 * 8 <
            g_nNearArenaSize_005d3050)
            g_pNearHeapAllocationPtr_005d305c =
                AllocateTaggedMemory(g_nNearArenaSize_005d3050, 0);
        if (g_pNearHeapAllocationPtr_005d305c != 0) {
            g_nNearHeapActive_00493044++;
            g_nNearHeapBase_005d3054 =
                DosFarPtrToNear(g_pNearHeapAllocationPtr_005d305c);
            if (*(short *)0x00400013 == 0x270) {
                g_nNearHeapRelocationBytes_0049304c =
                    0x9c000 - g_nNearHeapBase_005d3054 -
                    g_nNearArenaSize_005d3050;
                g_nNearArenaSize_005d3050 =
                    0x98000 - g_nNearHeapBase_005d3054 -
                    g_nNearHeapRelocationBytes_0049304c;
                if (g_nNearHeapMaxDescriptors_00493048 * 8 <
                    g_nNearArenaSize_005d3050) {
                    DosMemcpy(
                        DosNearPtrToFar(g_nNearArenaSize_005d3050 +
                                        g_nNearHeapBase_005d3054),
                        DosNearPtrToFar(
                            0x9c000 - g_nNearHeapRelocationBytes_0049304c),
                        g_nNearHeapRelocationBytes_0049304c);
                } else {
                    FreeIfNotNull(g_pNearHeapAllocationPtr_005d305c);
                    g_nNearHeapActive_00493044 = 0;
                }
            }
            if (g_nNearHeapActive_00493044 != 0)
                PurgeNearHeapBlocks(0x10);
        }
    }
    return g_nNearHeapActive_00493044;
}

/* Function start: 0x420B12 */
void ShutdownNearHeap(void)
{
    if (g_nNearHeapRelocationBytes_0049304c != 0) {
        DosMemcpy(
            DosNearPtrToFar(
                0x9c000 - g_nNearHeapRelocationBytes_0049304c),
            DosNearPtrToFar(
                g_nNearArenaSize_005d3050 +
                g_nNearHeapBase_005d3054),
            g_nNearHeapRelocationBytes_0049304c);
    }
    if (g_nNearHeapActive_00493044 != 0)
        FreeIfNotNull(g_pNearHeapAllocationPtr_005d305c);
    g_nNearHeapActive_00493044 = 0;
}

/* Function start: 0x420B88 */
void *AllocateNearHeapBlockFromEnd(int size, short flags)
{
    unsigned int newFlags;
    int freeSize;
    NearHeapBlock *entry;
    NearHeapBlock *lastBlock;
    int descriptorAddress;
    int moveAddress;
    int found;

    if (size <= 0)
        return 0;
    newFlags = 0x80000000;
    if ((flags & 3) == 1) {
        newFlags |= 0x10000000;
        size++;
    } else if ((flags & 3) == 2) {
        newFlags |= 0x20000000;
        size += 0xf;
    }
    if ((flags & 0x10) != 0)
        newFlags |= 0x40000000;

    found = 0;
    descriptorAddress =
        g_nNearHeapBase_005d3054 + g_nNearArenaSize_005d3050 - 8;
    while (descriptorAddress >= g_nNearHeapFirstDescriptor_005d3058) {
        entry = DosNearPtrToFar(descriptorAddress);
        freeSize = entry->sizeAndFlags & 0xfffff;
        if ((entry->sizeAndFlags & 0x80000000) == 0 && freeSize >= size) {
            if (freeSize > size) {
                if (g_nNearHeapBase_005d3054 + g_nNearArenaSize_005d3050 -
                        g_nNearHeapMaxDescriptors_00493048 * 8 >=
                    g_nNearHeapFirstDescriptor_005d3058) {
                    lastBlock = DosNearPtrToFar(
                        g_nNearHeapBase_005d3054 +
                        g_nNearArenaSize_005d3050 - 8);
                    if ((lastBlock->sizeAndFlags & 0x80000000) == 0 &&
                        lastBlock->address +
                                (lastBlock->sizeAndFlags & 0xfffff) ==
                            g_nNearHeapFirstDescriptor_005d3058 &&
                        (int)(lastBlock->sizeAndFlags & 0xfffff) > 8) {
                        lastBlock->sizeAndFlags -= 8;
                        g_nNearHeapMaxDescriptors_00493048++;
                    }
                }
                if (g_nNearHeapBase_005d3054 + g_nNearArenaSize_005d3050 -
                        g_nNearHeapMaxDescriptors_00493048 * 8 <
                    g_nNearHeapFirstDescriptor_005d3058) {
                    g_nNearHeapFirstDescriptor_005d3058 -= 8;
                    moveAddress = g_nNearHeapFirstDescriptor_005d3058;
                    while (moveAddress < descriptorAddress) {
                        lastBlock = DosNearPtrToFar(moveAddress);
                        lastBlock->address = lastBlock[1].address;
                        lastBlock->sizeAndFlags = lastBlock[1].sizeAndFlags;
                        moveAddress += 8;
                    }
                    descriptorAddress -= 8;
                    entry = DosNearPtrToFar(descriptorAddress);
                    entry->sizeAndFlags -= size;
                    entry[1].address +=
                        (int)(entry->sizeAndFlags & 0xffffff);
                    entry[1].sizeAndFlags = newFlags + size;
                    found = entry[1].address;
                    break;
                }
            } else {
                entry->sizeAndFlags |= newFlags;
                found = entry->address;
                break;
            }
        }
        descriptorAddress -= 8;
    }

    if (found != 0) {
        if ((flags & 3) == 1)
            found = (found + 1) & 0xfffffffe;
        else if ((flags & 3) == 2)
            found = (found + 0xf) & 0xfffffff0;
    }
    return DosNearPtrToFar(found);
}

/* Function start: 0x420E16 */
void *AllocateNearHeapBlockByFlags(int size, short flags)
{
    NearHeapBlock *entry;
    NearHeapBlock *lastBlock;
    int descriptorAddress;
    int moveAddress;
    int found;
    unsigned int freeSize;
    unsigned int newFlags;

    if ((flags & 0x20) != 0)
        return AllocateNearHeapBlockFromEnd(size, flags);
    if (size <= 0)
        return 0;
    newFlags = 0x80000000;
    if ((flags & 3) == 1) {
        newFlags |= 0x10000000;
        size++;
    } else if ((flags & 3) == 2) {
        newFlags |= 0x20000000;
        size += 0xf;
    }
    if ((flags & 0x10) != 0)
        newFlags |= 0x40000000;

    found = 0;
    descriptorAddress = g_nNearHeapFirstDescriptor_005d3058;
    while (descriptorAddress <
           g_nNearHeapBase_005d3054 + g_nNearArenaSize_005d3050) {
        entry = DosNearPtrToFar(descriptorAddress);
        freeSize = entry->sizeAndFlags & 0xfffff;
        if ((entry->sizeAndFlags & 0x80000000) == 0 &&
            size <= (int)freeSize) {
            if (size < (int)freeSize) {
                if (g_nNearHeapBase_005d3054 + g_nNearArenaSize_005d3050 -
                        g_nNearHeapMaxDescriptors_00493048 * 8 >=
                    g_nNearHeapFirstDescriptor_005d3058) {
                    lastBlock = DosNearPtrToFar(
                        g_nNearHeapBase_005d3054 +
                        g_nNearArenaSize_005d3050 - 8);
                    if ((lastBlock->sizeAndFlags & 0x80000000) == 0 &&
                        lastBlock->address +
                                (lastBlock->sizeAndFlags & 0xfffff) ==
                            g_nNearHeapFirstDescriptor_005d3058 &&
                        (int)(lastBlock->sizeAndFlags & 0xfffff) > 8) {
                        lastBlock->sizeAndFlags -= 8;
                        g_nNearHeapMaxDescriptors_00493048++;
                    }
                }
                if (g_nNearHeapBase_005d3054 + g_nNearArenaSize_005d3050 -
                        g_nNearHeapMaxDescriptors_00493048 * 8 <
                    g_nNearHeapFirstDescriptor_005d3058) {
                    g_nNearHeapFirstDescriptor_005d3058 -= 8;
                    moveAddress = g_nNearHeapFirstDescriptor_005d3058;
                    while (moveAddress < descriptorAddress) {
                        lastBlock = DosNearPtrToFar(moveAddress);
                        lastBlock->address = lastBlock[1].address;
                        lastBlock->sizeAndFlags = lastBlock[1].sizeAndFlags;
                        moveAddress += 8;
                    }
                    descriptorAddress -= 8;
                    entry = DosNearPtrToFar(descriptorAddress);
                    entry->sizeAndFlags = size + newFlags;
                    entry[1].address += size;
                    entry[1].sizeAndFlags -= size;
                    found = entry->address;
                    break;
                }
            } else {
                entry->sizeAndFlags |= newFlags;
                found = entry->address;
                break;
            }
        }
        descriptorAddress += 8;
    }

    if (found != 0) {
        if ((flags & 3) == 1)
            found = (found + 1) & 0xfffffffe;
        else if ((flags & 3) == 2)
            found = (found + 0xf) & 0xfffffff0;
    }
    return DosNearPtrToFar(found);
}

/* Function start: 0x4210B8 */
/* Nothing in the shipped executable calls this.  Releases the near-heap
 * block whose extent contains pointer, searching the descriptors from the
 * top of the heap down. */
void FreeNearHeapBlock(void *pointer)
{
    int address;
    NearHeapBlock *descriptor;
    int descriptorAddress;

    address = DosFarPtrToNear(pointer);
    descriptorAddress =
        g_nNearArenaSize_005d3050 + g_nNearHeapBase_005d3054 - 8;
    while (descriptorAddress >= g_nNearHeapFirstDescriptor_005d3058) {
        descriptor = DosNearPtrToFar(descriptorAddress);
        if (descriptor->address <= address &&
            (int)(descriptor->sizeAndFlags & 0xfffff) +
                    descriptor->address > address) {
            ReleaseNearHeapBlock(descriptorAddress);
            break;
        }
        descriptorAddress -= 8;
    }
}

/* Function start: 0x421144 */
int GetNearHeapBlockSize(void *pointer)
{
    int descriptorAddress;
    int address;
    int size;
    NearHeapBlock *descriptor;

    size = 0;
    address = DosFarPtrToNear(pointer);
    descriptorAddress =
        g_nNearArenaSize_005d3050 + g_nNearHeapBase_005d3054 - 8;
    while (g_nNearHeapFirstDescriptor_005d3058 <= descriptorAddress) {
        descriptor = DosNearPtrToFar(descriptorAddress);
        if (descriptor->address <= address &&
            descriptor->address +
                    (int)(descriptor->sizeAndFlags & 0xfffff) >
                address) {
            size = descriptor->sizeAndFlags & 0xfffff;
            /* Each tag flag reserves bytes the caller cannot use. */
            if ((descriptor->sizeAndFlags & 0x10000000) != 0)
                size--;
            else if ((descriptor->sizeAndFlags & 0x20000000) != 0)
                size -= 15;
            break;
        }
        descriptorAddress -= 8;
    }
    return size;
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
volatile int g_nNearArenaSize_005d3050;
volatile int g_nNearHeapBase_005d3054;
int g_nNearHeapFirstDescriptor_005d3058;
void * volatile g_pNearHeapAllocationPtr_005d305c;
