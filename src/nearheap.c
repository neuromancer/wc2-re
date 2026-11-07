/*
 *  Near heap allocator.
 *
 *  Retail object 0x4207B0-0x42120F (link order; see docs/ORDER.md).
 */
#define NEARHEAP_C
#include "game.h"

volatile short g_nNearHeapActive = 0; /* 0x00493044 */
volatile short g_nNearHeapMaxDescriptors = 0x80; /* 0x00493048 */
volatile int g_nNearHeapRelocationBytes = 0; /* 0x0049304c */
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
        while (descriptorAddress > g_nNearHeapFirstDescriptor) {
            descriptorAddress -= 8;
            block = DosNearPtrToFar(descriptorAddress);
            block[1].address = block->address;
            block[1].sizeAndFlags = block->sizeAndFlags;
        }
        g_nNearHeapFirstDescriptor += 8;
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
    if (g_nNearArenaSize + g_1_nNearHeapBase >
            descriptorAddress + 8 &&
        MergeAdjacentNearHeapBlocks(descriptorAddress) != 0)
        descriptorAddress += 8;
    if (descriptorAddress > g_nNearHeapFirstDescriptor)
        MergeAdjacentNearHeapBlocks(descriptorAddress - 8);
    return descriptorAddress;
}

/* Function start: 0x4208FC */
void PurgeNearHeapBlocks(short flags)
{
    NearHeapBlock *entry;
    int descriptorAddress;

    if (g_nNearHeapActive != 0) {
        if ((flags & 0x10) != 0) {
            g_nNearHeapFirstDescriptor =
                g_nNearArenaSize + g_1_nNearHeapBase - 8;
            entry = DosNearPtrToFar(g_nNearHeapFirstDescriptor);
            entry->address = g_1_nNearHeapBase;
            entry->sizeAndFlags = g_nNearArenaSize -
                                  g_nNearHeapMaxDescriptors * 8;
        } else {
            descriptorAddress =
                g_nNearArenaSize + g_1_nNearHeapBase - 8;
            while (descriptorAddress >=
                   g_nNearHeapFirstDescriptor) {
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
    if (g_nNearHeapActive == 0) {
        g_nNearArenaSize = (short)GetNavRangeSentinel();
        g_1_pNearHeapAllocationPtr = 0;
        if (g_nNearHeapMaxDescriptors * 8 <
            g_nNearArenaSize)
            g_1_pNearHeapAllocationPtr =
                AllocateTaggedMemory(g_nNearArenaSize, 0);
        if (g_1_pNearHeapAllocationPtr != 0) {
            g_nNearHeapActive++;
            g_1_nNearHeapBase =
                DosFarPtrToNear(g_1_pNearHeapAllocationPtr);
            if (*(short *)0x00400013 == 0x270) {
                g_nNearHeapRelocationBytes =
                    0x9c000 - g_1_nNearHeapBase -
                    g_nNearArenaSize;
                g_nNearArenaSize =
                    0x98000 - g_1_nNearHeapBase -
                    g_nNearHeapRelocationBytes;
                if (g_nNearHeapMaxDescriptors * 8 <
                    g_nNearArenaSize) {
                    DosMemmove(
                        DosNearPtrToFar(g_nNearArenaSize +
                                        g_1_nNearHeapBase),
                        DosNearPtrToFar(
                            0x9c000 - g_nNearHeapRelocationBytes),
                        g_nNearHeapRelocationBytes);
                } else {
                    FreeIfNotNull(g_1_pNearHeapAllocationPtr);
                    g_nNearHeapActive = 0;
                }
            }
            if (g_nNearHeapActive != 0)
                PurgeNearHeapBlocks(0x10);
        }
    }
    return g_nNearHeapActive;
}

/* Function start: 0x420B12 */
void ShutdownNearHeap(void)
{
    if (g_nNearHeapRelocationBytes != 0) {
        DosMemmove(
            DosNearPtrToFar(
                0x9c000 - g_nNearHeapRelocationBytes),
            DosNearPtrToFar(
                g_nNearArenaSize +
                g_1_nNearHeapBase),
            g_nNearHeapRelocationBytes);
    }
    if (g_nNearHeapActive != 0)
        FreeIfNotNull(g_1_pNearHeapAllocationPtr);
    g_nNearHeapActive = 0;
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
        g_1_nNearHeapBase + g_nNearArenaSize - 8;
    while (descriptorAddress >= g_nNearHeapFirstDescriptor) {
        entry = DosNearPtrToFar(descriptorAddress);
        freeSize = entry->sizeAndFlags & 0xfffff;
        if ((entry->sizeAndFlags & 0x80000000) == 0 && freeSize >= size) {
            if (freeSize > size) {
                if (g_1_nNearHeapBase + g_nNearArenaSize -
                        g_nNearHeapMaxDescriptors * 8 >=
                    g_nNearHeapFirstDescriptor) {
                    lastBlock = DosNearPtrToFar(
                        g_1_nNearHeapBase +
                        g_nNearArenaSize - 8);
                    if ((lastBlock->sizeAndFlags & 0x80000000) == 0 &&
                        lastBlock->address +
                                (lastBlock->sizeAndFlags & 0xfffff) ==
                            g_nNearHeapFirstDescriptor &&
                        (int)(lastBlock->sizeAndFlags & 0xfffff) > 8) {
                        lastBlock->sizeAndFlags -= 8;
                        g_nNearHeapMaxDescriptors++;
                    }
                }
                if (g_1_nNearHeapBase + g_nNearArenaSize -
                        g_nNearHeapMaxDescriptors * 8 <
                    g_nNearHeapFirstDescriptor) {
                    g_nNearHeapFirstDescriptor -= 8;
                    moveAddress = g_nNearHeapFirstDescriptor;
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
    descriptorAddress = g_nNearHeapFirstDescriptor;
    while (descriptorAddress <
           g_1_nNearHeapBase + g_nNearArenaSize) {
        entry = DosNearPtrToFar(descriptorAddress);
        freeSize = entry->sizeAndFlags & 0xfffff;
        if ((entry->sizeAndFlags & 0x80000000) == 0 &&
            size <= (int)freeSize) {
            if (size < (int)freeSize) {
                if (g_1_nNearHeapBase + g_nNearArenaSize -
                        g_nNearHeapMaxDescriptors * 8 >=
                    g_nNearHeapFirstDescriptor) {
                    lastBlock = DosNearPtrToFar(
                        g_1_nNearHeapBase +
                        g_nNearArenaSize - 8);
                    if ((lastBlock->sizeAndFlags & 0x80000000) == 0 &&
                        lastBlock->address +
                                (lastBlock->sizeAndFlags & 0xfffff) ==
                            g_nNearHeapFirstDescriptor &&
                        (int)(lastBlock->sizeAndFlags & 0xfffff) > 8) {
                        lastBlock->sizeAndFlags -= 8;
                        g_nNearHeapMaxDescriptors++;
                    }
                }
                if (g_1_nNearHeapBase + g_nNearArenaSize -
                        g_nNearHeapMaxDescriptors * 8 <
                    g_nNearHeapFirstDescriptor) {
                    g_nNearHeapFirstDescriptor -= 8;
                    moveAddress = g_nNearHeapFirstDescriptor;
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
        g_nNearArenaSize + g_1_nNearHeapBase - 8;
    while (descriptorAddress >= g_nNearHeapFirstDescriptor) {
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
        g_nNearArenaSize + g_1_nNearHeapBase - 8;
    while (g_nNearHeapFirstDescriptor <= descriptorAddress) {
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
volatile int g_nNearArenaSize; /* 0x005d3050 */
volatile int g_1_nNearHeapBase; /* 0x005d3054 */
int g_nNearHeapFirstDescriptor; /* 0x005d3058 */
void * volatile g_1_pNearHeapAllocationPtr; /* 0x005d305c */
