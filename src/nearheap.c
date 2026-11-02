/*
 *  Near heap allocator.
 *
 *  Retail object 0x4207B0-0x42120F (link order; see docs/ORDER.md).
 */
#include "game.h"
#include <float.h>

volatile short g_nNearHeapActive_00493044 = 0;

volatile short g_nNearHeapMaxDescriptors_00493048 = 0x80;

volatile int g_nNearHeapRelocationBytes_0049304c = 0;

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
    if (g_nNearHeapSize_005d3050 + g_nNearHeapBase_005d3054 >
            descriptorAddress + 8 &&
        MergeAdjacentNearHeapBlocks(descriptorAddress) != 0)
        descriptorAddress += 8;
    if (descriptorAddress > g_nNearHeapFirstDescriptor_005d3058)
        MergeAdjacentNearHeapBlocks(descriptorAddress - 8);
    return descriptorAddress;
}

/* Function start: 0x4208FC */
void PurgeNearHeapBlocks(unsigned short flags)
{
    NearHeapBlock *block;
    int descriptorAddress;
    int descriptorBytes;

    if (g_nNearHeapActive_00493044 != 0) {
        if ((flags & 0x10) != 0) {
            descriptorAddress = g_nNearHeapBase_005d3054;
            descriptorAddress += g_nNearHeapSize_005d3050;
            descriptorAddress -= 8;
            g_nNearHeapFirstDescriptor_005d3058 = descriptorAddress;
            block = DosNearPtrToFar(descriptorAddress);
            block->address = g_nNearHeapBase_005d3054;
            descriptorBytes = g_nNearHeapMaxDescriptors_00493048 * 8;
            block->sizeAndFlags =
                g_nNearHeapSize_005d3050 - descriptorBytes;
            return;
        }
        descriptorAddress = g_nNearHeapBase_005d3054 +
                            g_nNearHeapSize_005d3050 - 8;
        for (; descriptorAddress >= g_nNearHeapFirstDescriptor_005d3058;
             descriptorAddress -= 8) {
            block = DosNearPtrToFar(descriptorAddress);
            if ((block->sizeAndFlags & 0x40000000) == 0)
                descriptorAddress = ReleaseNearHeapBlock(descriptorAddress);
        }
    }
}

/* Function start: 0x4209C2 */
unsigned short InitializeNearHeap(void)
{
    short initialSize;
    int adjustedSize;

    if (g_nNearHeapActive_00493044 == 0) {
        initialSize = (short)GetNavRangeSentinel();
        g_pNearHeapAllocation_005d305c = 0;
        g_nNearHeapSize_005d3050 = initialSize;
        if (g_nNearHeapMaxDescriptors_00493048 * 8 <
            g_nNearHeapSize_005d3050) {
            g_pNearHeapAllocation_005d305c =
                AllocateTaggedMemory(g_nNearHeapSize_005d3050, 0);
            if (g_pNearHeapAllocation_005d305c != 0) {
                g_nNearHeapActive_00493044++;
                g_nNearHeapBase_005d3054 =
                    DosFarPtrToNear(g_pNearHeapAllocation_005d305c);
                if (*(unsigned short *)0x00400013 == 0x270) {
                    g_nNearHeapRelocationBytes_0049304c =
                        0x9c000 - g_nNearHeapSize_005d3050 -
                        g_nNearHeapBase_005d3054;
                    adjustedSize =
                        0x98000 - g_nNearHeapRelocationBytes_0049304c;
                    adjustedSize -= g_nNearHeapBase_005d3054;
                    g_nNearHeapSize_005d3050 = adjustedSize;
                    if (g_nNearHeapMaxDescriptors_00493048 * 8 <
                        g_nNearHeapSize_005d3050) {
                        DosMemcpy(
                            DosNearPtrToFar(
                                g_nNearHeapBase_005d3054 +
                                g_nNearHeapSize_005d3050),
                            DosNearPtrToFar(
                                0x9c000 -
                                g_nNearHeapRelocationBytes_0049304c),
                            g_nNearHeapRelocationBytes_0049304c);
                    } else {
                        FreeIfNotNull(g_pNearHeapAllocation_005d305c);
                        g_nNearHeapActive_00493044 = 0;
                    }
                }
                if (g_nNearHeapActive_00493044 != 0)
                    PurgeNearHeapBlocks(0x10);
            }
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
                g_nNearHeapSize_005d3050 +
                g_nNearHeapBase_005d3054),
            g_nNearHeapRelocationBytes_0049304c);
    }
    if (g_nNearHeapActive_00493044 != 0)
        FreeIfNotNull(g_pNearHeapAllocation_005d305c);
    g_nNearHeapActive_00493044 = 0;
}

/* Function start: 0x420B88 */
void *AllocateNearHeapBlockFromEnd(int size, unsigned short flags)
{
#if 1
    NearHeapBlock *block;
    NearHeapBlock *lastBlock;
    int descriptorAddress;
    int shiftAddress;
    int allocationAddress;
    int blockSize;
    unsigned int allocationFlags;

    if (size <= 0)
        return 0;
    allocationFlags = 0x80000000;
    if ((flags & 3) == 1) {
        allocationFlags |= 0x10000000;
        size++;
    } else if ((flags & 3) == 2) {
        allocationFlags |= 0x20000000;
        size += 0xf;
    }
    if ((flags & 0x10) != 0)
        allocationFlags |= 0x40000000;

    allocationAddress = 0;
    descriptorAddress =
        g_nNearHeapBase_005d3054 + g_nNearHeapSize_005d3050 - 8;
    for (; descriptorAddress >= g_nNearHeapFirstDescriptor_005d3058;
         descriptorAddress -= 8) {
        block = DosNearPtrToFar(descriptorAddress);
        blockSize = block->sizeAndFlags & 0xfffff;
        if ((block->sizeAndFlags & 0x80000000) != 0)
            continue;
        if ((int)blockSize < size)
            continue;
        if ((int)blockSize > size) {
            if (g_nNearHeapBase_005d3054 -
                    g_nNearHeapMaxDescriptors_00493048 * 8 +
                    g_nNearHeapSize_005d3050 >=
                g_nNearHeapFirstDescriptor_005d3058) {
                lastBlock = DosNearPtrToFar(
                    g_nNearHeapBase_005d3054 +
                    g_nNearHeapSize_005d3050 - 8);
                if ((lastBlock->sizeAndFlags & 0x80000000) == 0) {
                    blockSize = lastBlock->sizeAndFlags & 0xfffff;
                    if (lastBlock->address + blockSize ==
                            g_nNearHeapFirstDescriptor_005d3058 &&
                        (int)blockSize > 8) {
                        lastBlock->sizeAndFlags -= 8;
                        g_nNearHeapMaxDescriptors_00493048++;
                    }
                }
            }

            if (g_nNearHeapBase_005d3054 -
                    g_nNearHeapMaxDescriptors_00493048 * 8 +
                    g_nNearHeapSize_005d3050 >=
                g_nNearHeapFirstDescriptor_005d3058)
                continue;

            g_nNearHeapFirstDescriptor_005d3058 -= 8;
            shiftAddress = g_nNearHeapFirstDescriptor_005d3058;
            for (; descriptorAddress > shiftAddress; shiftAddress += 8) {
                block = DosNearPtrToFar(shiftAddress);
                block->address = block[1].address;
                block->sizeAndFlags = block[1].sizeAndFlags;
            }
            descriptorAddress -= 8;
            block = DosNearPtrToFar(descriptorAddress);
            blockSize = block->sizeAndFlags - size;
            block->sizeAndFlags = blockSize;
            block[1].address += blockSize & 0xffffff;
            blockSize = allocationFlags + size;
            block[1].sizeAndFlags = blockSize;
            allocationAddress = block[1].address;
            break;
        }
        block->sizeAndFlags |= allocationFlags;
        allocationAddress = block->address;
        break;
    }

    if (allocationAddress != 0) {
        if ((flags & 3) == 1)
            allocationAddress = (allocationAddress + 1) & 0xfffffffe;
        else if ((flags & 3) == 2)
            allocationAddress = (allocationAddress + 0xf) & 0xfffffff0;
    }
    return DosNearPtrToFar(allocationAddress);
#else
    NearHeapBlock *block;
    NearHeapBlock *lastBlock;
    int descriptorAddress;
    int shiftAddress;
    unsigned int allocationAddress;
    unsigned int allocationFlags;

    if (size <= 0)
        return 0;
    allocationFlags = 0x80000000;
    if ((flags & 3) == 1) {
        allocationFlags = 0x90000000;
        size++;
    } else if ((flags & 3) == 2) {
        allocationFlags = 0xa0000000;
        size += 0xf;
    }
    if ((flags & 0x10) != 0)
        allocationFlags |= 0x40000000;

    allocationAddress = 0;
    descriptorAddress = g_nNearHeapBase_005d3054 +
                        g_nNearHeapSize_005d3050;
    do {
        descriptorAddress -= 8;
        if (descriptorAddress < g_nNearHeapFirstDescriptor_005d3058)
            goto allocation_complete;
        block = DosNearPtrToFar(descriptorAddress);
        if ((block->sizeAndFlags & 0x80000000) != 0 ||
            (int)(block->sizeAndFlags & 0xfffff) < size)
            continue;
        if ((int)(block->sizeAndFlags & 0xfffff) <= size) {
            block->sizeAndFlags |= allocationFlags;
            allocationAddress = block->address;
            goto allocation_complete;
        }
        if (g_nNearHeapFirstDescriptor_005d3058 <=
                g_nNearHeapBase_005d3054 + g_nNearHeapSize_005d3050 -
                    g_nNearHeapMaxDescriptors_00493048 * 8) {
            lastBlock = DosNearPtrToFar(
                g_nNearHeapBase_005d3054 + g_nNearHeapSize_005d3050 - 8);
            if ((lastBlock->sizeAndFlags & 0x80000000) == 0 &&
                (lastBlock->sizeAndFlags & 0xfffff) + lastBlock->address ==
                    g_nNearHeapFirstDescriptor_005d3058 &&
                (lastBlock->sizeAndFlags & 0xfffff) > 8) {
                lastBlock->sizeAndFlags -= 8;
                g_nNearHeapMaxDescriptors_00493048++;
            }
        }
    } while (g_nNearHeapFirstDescriptor_005d3058 <=
             g_nNearHeapBase_005d3054 + g_nNearHeapSize_005d3050 -
                 g_nNearHeapMaxDescriptors_00493048 * 8);

    g_nNearHeapFirstDescriptor_005d3058 -= 8;
    for (shiftAddress = g_nNearHeapFirstDescriptor_005d3058;
         shiftAddress < descriptorAddress; shiftAddress += 8) {
        block = DosNearPtrToFar(shiftAddress);
        block->address = block[1].address;
        block->sizeAndFlags = block[1].sizeAndFlags;
    }
    block = DosNearPtrToFar(descriptorAddress - 8);
    block->sizeAndFlags -= size;
    block[1].address += block->sizeAndFlags & 0xffffff;
    block[1].sizeAndFlags = allocationFlags + size;
    allocationAddress = block[1].address;

allocation_complete:
    if (allocationAddress != 0) {
        if ((flags & 3) == 1)
            allocationAddress = (allocationAddress + 1) & 0xfffffffe;
        else if ((flags & 3) == 2)
            allocationAddress = (allocationAddress + 0xf) & 0xfffffff0;
    }
    return DosNearPtrToFar(allocationAddress);
#endif
}

/* Function start: 0x420E16 */
void *AllocateNearHeapBlockByFlags(int size, unsigned short flags)
{
    NearHeapBlock *block;
    NearHeapBlock *lastBlock;
    int descriptorAddress;
    int shiftAddress;
    int allocationAddress;
    unsigned int blockSize;
    unsigned int allocationFlags;
    unsigned short alignment;

    if ((flags & 0x20) != 0)
        return AllocateNearHeapBlockFromEnd(size, flags);
    if (size <= 0)
        return 0;
    allocationFlags = 0x80000000;
    alignment = flags & 3;
    if (alignment == 1) {
        allocationFlags = 0x90000000;
        size++;
    } else if (alignment == 2) {
        allocationFlags = 0xa0000000;
        size += 0xf;
    }
    if ((flags & 0x10) != 0)
        allocationFlags |= 0x40000000;

    allocationAddress = 0;
    descriptorAddress = g_nNearHeapFirstDescriptor_005d3058;
    for (; descriptorAddress <
               g_nNearHeapBase_005d3054 + g_nNearHeapSize_005d3050;
         descriptorAddress += 8) {
        block = DosNearPtrToFar(descriptorAddress);
        blockSize = block->sizeAndFlags & 0xfffff;
        if ((block->sizeAndFlags & 0x80000000) != 0)
            continue;
        if ((int)blockSize < size)
            continue;
        if ((int)blockSize > size) {
            if (g_nNearHeapBase_005d3054 -
                    g_nNearHeapMaxDescriptors_00493048 * 8 +
                    g_nNearHeapSize_005d3050 >=
                g_nNearHeapFirstDescriptor_005d3058) {
                lastBlock = DosNearPtrToFar(
                    g_nNearHeapBase_005d3054 +
                    g_nNearHeapSize_005d3050 - 8);
                if ((lastBlock->sizeAndFlags & 0x80000000) == 0) {
                    blockSize = lastBlock->sizeAndFlags & 0xfffff;
                    if (lastBlock->address + blockSize ==
                            g_nNearHeapFirstDescriptor_005d3058 &&
                        (int)blockSize > 8) {
                        lastBlock->sizeAndFlags -= 8;
                        g_nNearHeapMaxDescriptors_00493048++;
                    }
                }
            }

            if (g_nNearHeapBase_005d3054 -
                    g_nNearHeapMaxDescriptors_00493048 * 8 +
                    g_nNearHeapSize_005d3050 >=
                g_nNearHeapFirstDescriptor_005d3058)
                continue;

            g_nNearHeapFirstDescriptor_005d3058 -= 8;
            shiftAddress = g_nNearHeapFirstDescriptor_005d3058;
            for (; descriptorAddress > shiftAddress; shiftAddress += 8) {
                block = DosNearPtrToFar(shiftAddress);
                block->address = block[1].address;
                block->sizeAndFlags = block[1].sizeAndFlags;
            }
            descriptorAddress -= 8;
            block = DosNearPtrToFar(descriptorAddress);
            block->sizeAndFlags = allocationFlags + size;
            block[1].address += size;
            block[1].sizeAndFlags -= size;
            allocationAddress = block->address;
            break;
        }
        block->sizeAndFlags |= allocationFlags;
        allocationAddress = block->address;
        break;
    }

    if (allocationAddress != 0) {
        if (alignment == 1)
            allocationAddress = (allocationAddress + 1) & 0xfffffffe;
        else if (alignment == 2)
            allocationAddress = (allocationAddress + 0xf) & 0xfffffff0;
    }
    return DosNearPtrToFar(allocationAddress);
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
        g_nNearHeapSize_005d3050 + g_nNearHeapBase_005d3054 - 8;
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
        g_nNearHeapSize_005d3050 + g_nNearHeapBase_005d3054 - 8;
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
