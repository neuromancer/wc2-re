/*
 *  Overlap-safe huge memory copy.
 *
 *  Retail object 0x419A40-0x419C0F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x419A40 */
void CopyHugeMemoryOverlapSafe(void *destination, void *source, int count)
{
#ifdef SDL_PORT
    /* See issue #37 (crash reloading a savegame): DosFarPtrToNear/
     * DosNearPtrToFar round-trip a pointer through "unsigned int" -- exactly
     * correct on the 32-bit reference build (a dword IS a pointer there,
     * see the DwordPtr comment in game.h), but on this 64-bit build it
     * silently truncates the upper 32 bits. The branch below reconstructs
     * both pointers through that round-trip unconditionally, and which
     * branch runs depends on comparing the two pointers' truncated low 32
     * bits -- so whether this corrupts the pointers effectively comes down
     * to heap-address luck, reproducing more often the more the heap has
     * churned (e.g. reopening the save/load menu a second time).
     *
     * The 0xffff-sized chunking in both branches below exists only to stay
     * within a DOS segment's 64KB limit; that does not apply on a flat
     * 64-bit address space, so a plain memmove is exactly equivalent and
     * correct regardless of overlap -- which is exactly what this
     * hand-rolled routine's name promises. */
    if (count != 0)
        memmove(destination, source, (size_t)count);
#else
    if (count == 0)
        return;
    if (DosFarPtrToNear(source) < DosFarPtrToNear(destination)) {
        destination = DosNearPtrToFar(DosFarPtrToNear(destination) + count);
        source = DosNearPtrToFar(DosFarPtrToNear(source) + count);
        for (;;) {
            if ((count -= 0xffff) > -1) {
                source = DosNearPtrToFar(DosFarPtrToNear(source) - 0xffff);
                destination = DosNearPtrToFar(
                    DosFarPtrToNear(destination) - 0xffff);
                DosMemmove(destination, source, 0xffff);
            } else
                break;
        }
        count += 0xffff;
        source = DosNearPtrToFar(DosFarPtrToNear(source) - count);
        destination = DosNearPtrToFar(DosFarPtrToNear(destination) - count);
        DosMemmove(destination, source, (unsigned int)count);
    } else {
        for (;;) {
            if ((count -= 0xffff) > -1) {
                DosMemmove(destination, source, 0xffff);
                source = DosNearPtrToFar(DosFarPtrToNear(source) + 0xffff);
                destination = DosNearPtrToFar(
                    DosFarPtrToNear(destination) + 0xffff);
            } else
                break;
        }
        count += 0xffff;
        DosMemmove(destination, source, (unsigned int)count);
    }
#endif
}
