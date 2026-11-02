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
        if (g_nPacketHandleCount_005d1020 == 0x1000)
            exit_squadron("qq mem push overflow");
        g_aiPacketHandleOffsets_005cd020[
            g_nPacketHandleCount_005d1020] = offset;
        if (offset < 0) {
            memory = (unsigned char *)memory - offset;
            g_apPacketHandles_005c9020[
                g_nPacketHandleCount_005d1020] = memory;
        } else {
            memory = (unsigned char *)memory + offset;
            g_apPacketHandles_005c9020[
                g_nPacketHandleCount_005d1020] = memory;
        }
        g_nPacketHandleCount_005d1020++;
    }
    return memory;
}

/* Function start: 0x458EED */
int IsPushedPacketHandle(void *handle)
{
    int index;

    for (index = 0; index < g_nPacketHandleCount_005d1020; index++) {
        if (g_apPacketHandles_005c9020[index] == handle) {
            return g_aiPacketHandleOffsets_005cd020[index] < 0;
        }
    }
    return 0;
}

/* Function start: 0x458F5A */
void *MapPacketHandleToBlock(void *handle)
{
    int i;

    for (i = 0; i < g_nPacketHandleCount_005d1020; i++) {
        if (g_apPacketHandles_005c9020[i] == handle) {
            if (g_aiPacketHandleOffsets_005cd020[i] < 0)
                handle = (unsigned char *)handle +
                    g_aiPacketHandleOffsets_005cd020[i];
            else
                handle = (unsigned char *)handle -
                    g_aiPacketHandleOffsets_005cd020[i];
            g_nPacketHandleCount_005d1020--;
            g_apPacketHandles_005c9020[i] =
                g_apPacketHandles_005c9020[g_nPacketHandleCount_005d1020];
            g_aiPacketHandleOffsets_005cd020[i] =
                g_aiPacketHandleOffsets_005cd020[
                    g_nPacketHandleCount_005d1020];
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
               g_abTaggedAllocationPrefix_0049ca40,
               sizeof(g_abTaggedAllocationPrefix_0049ca40));
        memory = PushMemoryStackFrame(
            memory, -(int)(8 + sizeof(unsigned char *)));
#else
        memcpy(memory, g_abTaggedAllocationPrefix_0049ca40,
               sizeof(g_abTaggedAllocationPrefix_0049ca40));
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
        if (g_apPacketReferenceGroups_0049b898[group] != 0) {
            for (entry = 0; entry < 0x25; entry++) {
                if (g_apPacketReferenceGroups_0049b898[group][entry] ==
                    handle)
                    g_apPacketReferenceGroups_0049b898[group][entry] = 0;
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
