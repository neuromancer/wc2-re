/*
 *  Palette transition step.
 *
 *  Retail object 0x4225A0-0x4228AF (link order; see docs/ORDER.md).
 */
#include "game.h"

short g_nPaletteTransitionInitialise = 1; /* 0x0049305c */
#ifndef SDL_PORT
#include <malloc.h>
#endif

/* Function start: 0x4225A0 */
short StepPaletteTransition(short *current,
                            const short *target,
                            short componentCount)
{
    short colour;
    short difference;
    short previousCountdown;

    if (g_nPaletteTransitionInitialise != 0) {
        g_pPaletteTransitionAccumulator =
            AllocateTaggedMemory((unsigned int)(componentCount * 2), 0);
        g_pPaletteTransitionDeltaPtr =
            AllocateTaggedMemory((unsigned int)(componentCount * 2), 0);
        g_pPaletteTransitionDirectionRef =
            AllocateTaggedMemory((unsigned int)(componentCount * 2), 0);
        if (g_pPaletteTransitionAccumulator == 0 ||
            g_pPaletteTransitionDeltaPtr == 0 ||
            g_pPaletteTransitionDirectionRef == 0) {
            if (g_pPaletteTransitionAccumulator != 0)
                ReleasePacketHandle(g_pPaletteTransitionAccumulator);
            if (g_pPaletteTransitionDeltaPtr != 0)
                ReleasePacketHandle(g_pPaletteTransitionDeltaPtr);
            if (g_pPaletteTransitionDirectionRef != 0)
                ReleasePacketHandle(g_pPaletteTransitionDirectionRef);
            return 0;
        }

        g_2_nPaletteTransitionMaxDelta = 0;
        for (colour = 0; colour < componentCount; colour++) {
            difference = (short)(current[colour] - target[colour]);
            if (difference < 0) {
                difference = (short)-difference;
                g_pPaletteTransitionDirectionRef[colour] = 1;
            } else {
                g_pPaletteTransitionDirectionRef[colour] = -1;
            }
            g_pPaletteTransitionDeltaPtr[colour] = difference;
            if (g_2_nPaletteTransitionMaxDelta < difference)
                g_2_nPaletteTransitionMaxDelta = difference;
        }

        difference = (short)(g_2_nPaletteTransitionMaxDelta / 2);
        for (colour = 0; colour < componentCount; colour++)
            g_pPaletteTransitionAccumulator[colour] = difference;
        g_nPaletteTransitionCountdownNumber =
            g_2_nPaletteTransitionMaxDelta;
        g_nPaletteTransitionInitialise = 0;
    }

    previousCountdown = g_nPaletteTransitionCountdownNumber;
    g_nPaletteTransitionCountdownNumber--;
    if (previousCountdown == 0) {
        ReleasePacketHandle(g_pPaletteTransitionAccumulator);
        ReleasePacketHandle(g_pPaletteTransitionDeltaPtr);
        ReleasePacketHandle(g_pPaletteTransitionDirectionRef);
        g_nPaletteTransitionInitialise = 1;
        return 0;
    }

    for (colour = 0; colour < componentCount; colour++) {
        g_pPaletteTransitionAccumulator[colour] =
            (short)(g_pPaletteTransitionAccumulator[colour] +
                    g_pPaletteTransitionDeltaPtr[colour]);
        if (g_pPaletteTransitionAccumulator[colour] >
            g_2_nPaletteTransitionMaxDelta) {
            g_pPaletteTransitionAccumulator[colour] =
                (short)(g_pPaletteTransitionAccumulator[colour] -
                        g_2_nPaletteTransitionMaxDelta);
            current[colour] =
                (short)(current[colour] +
                        g_pPaletteTransitionDirectionRef[colour]);
        }
    }
    return 1;
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
short *g_pPaletteTransitionDirectionRef; /* 0x005d3010 */
short *g_pPaletteTransitionDeltaPtr; /* 0x005d3014 */
short g_2_nPaletteTransitionMaxDelta; /* 0x005d3018 */
short *g_pPaletteTransitionAccumulator; /* 0x005d301c */
short g_nPaletteTransitionCountdownNumber; /* 0x005d3020 */
