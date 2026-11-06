/*
 *  Memory stack frames and packet handles.
 *
 *  Retail object 0x458E10-0x45915F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x458E10 */
/* Nothing in the shipped executable calls this.  The DOS memory API's heap
 * reset; on Win32 the heap it resets is an empty hook. */
void ClearMemoryHeaps(void)
{
    TimerResetHook();
}

/* Function start: 0x458E20 */
unsigned short IsSoundHardwarePresent(short device)
{
    EnableHighMemoryResources();
    return 1;
}

/* Function start: 0x458E39 */
void MessagePumpHook(int mode)
{
    TimerStopHook();
}

/* Function start: 0x458E49 */
void *PushMemoryStackFrame(void *memory, int offset)
{
    if (offset != 0) {
#ifndef SDL_PORT
        printf("push %p by %d\n", memory, offset);
#endif
        if (g_nPacketHandleCount == 0x1000)
            exit_squadron("qq mem push overflow");
        g_aiPacketHandleOffsets[
            g_nPacketHandleCount] = offset;
        if (offset < 0) {
            memory = (unsigned char *)memory - offset;
            g_9_apPacketHandlesTable[
                g_nPacketHandleCount] = memory;
        } else {
            memory = (unsigned char *)memory + offset;
            g_9_apPacketHandlesTable[
                g_nPacketHandleCount] = memory;
        }
        g_nPacketHandleCount++;
    }
    return memory;
}

/* Function start: 0x458EED */
int IsPushedPacketHandle(void *handle)
{
    int index;

    for (index = 0; index < g_nPacketHandleCount; index++) {
        if (g_9_apPacketHandlesTable[index] == handle) {
            return g_aiPacketHandleOffsets[index] < 0;
        }
    }
    return 0;
}

/* Function start: 0x458F5A */
void *MapPacketHandleToBlock(void *handle)
{
    int i;

    for (i = 0; i < g_nPacketHandleCount; i++) {
        if (g_9_apPacketHandlesTable[i] == handle) {
            if (g_aiPacketHandleOffsets[i] < 0)
                handle = (unsigned char *)handle +
                    g_aiPacketHandleOffsets[i];
            else
                handle = (unsigned char *)handle -
                    g_aiPacketHandleOffsets[i];
            g_nPacketHandleCount--;
            g_9_apPacketHandlesTable[i] =
                g_9_apPacketHandlesTable[g_nPacketHandleCount];
            g_aiPacketHandleOffsets[i] =
                g_aiPacketHandleOffsets[
                    g_nPacketHandleCount];
            return MapPacketHandleToBlock(handle);
        }
    }
    return handle;
}

#pragma function(memcpy)
/* Function start: 0x45901D */
void *AllocateTaggedMemory(unsigned int size, short flags)
{
    void *memory;

#ifdef SDL_PORT
    if ((flags & 0x40) != 0)
        size += 8 + sizeof(unsigned char *);
#else
    if ((flags & 0x40) != 0)
        size += 8;
#endif
    memory = AllocateGuardedMemory(size);
    if ((flags & 0x40) != 0) {
#ifdef SDL_PORT
        *(unsigned char **)memory = 0;
        memcpy((unsigned char *)memory + sizeof(unsigned char *),
               g_abTaggedAllocationPrefix,
               sizeof(g_abTaggedAllocationPrefix));
        memory = PushMemoryStackFrame(
            memory, -(int)(8 + sizeof(unsigned char *)));
#else
        memcpy(memory, g_abTaggedAllocationPrefix,
               sizeof(g_abTaggedAllocationPrefix));
        memory = PushMemoryStackFrame(memory, -8);
#endif
    }
    return memory;
}

/* Function start: 0x459082 */
void ReleasePacketHandle(void *handle)
{
    int group;
    int entry;

    if (handle == 0)
        return;
    for (group = 0; group < 4; group++) {
        if (g_apPacketReferenceGroups[group] != 0) {
            for (entry = 0; entry < 0x25; entry++) {
                if (g_apPacketReferenceGroups[group][entry] ==
                    handle)
                    g_apPacketReferenceGroups[group][entry] = 0;
            }
        }
    }
    handle = MapPacketHandleToBlock(handle);
    FreeGuardedAllocation(handle);
}

/* Function start: 0x459134 */
/* Tail-jump thunk to 0x004362E0, not a constant return -- Ghidra followed the
 * jump and folded the callee's body into the display.  The optimized compiler
 * emits the tail jump from this forwarding return. */
unsigned int GetAvailableFarMemoryByType(short memoryType)
{
    (void)memoryType;
    return GetAvailableFarMemory();
}

/* Function start: 0x459149 */
unsigned int GetLargestFreeMemoryBlockByType(short memoryType)
{
    (void)memoryType;
    return GetLargestFreeMemoryBlock();
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
void *g_9_apPacketHandlesTable[0x1000]; /* 0x005c9020 */
int g_aiPacketHandleOffsets[0x1000]; /* 0x005cd020 */
int g_nPacketHandleCount; /* 0x005d1020 */
