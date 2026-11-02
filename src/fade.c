/*
 *  Palette transition step.
 *
 *  Retail object 0x4225A0-0x4228AF (link order; see docs/ORDER.md).
 */
#include "game.h"
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

    if (g_nPaletteTransitionInitialise_0049305c != 0) {
        g_pPaletteTransitionAccumulator_005d301c =
            AllocateTaggedMemory((unsigned int)(componentCount * 2), 0);
        g_pPaletteTransitionDelta_005d3014 =
            AllocateTaggedMemory((unsigned int)(componentCount * 2), 0);
        g_pPaletteTransitionDirection_005d3010 =
            AllocateTaggedMemory((unsigned int)(componentCount * 2), 0);
        if (g_pPaletteTransitionAccumulator_005d301c == 0 ||
            g_pPaletteTransitionDelta_005d3014 == 0 ||
            g_pPaletteTransitionDirection_005d3010 == 0) {
            if (g_pPaletteTransitionAccumulator_005d301c != 0)
                ReleasePacketHandle(g_pPaletteTransitionAccumulator_005d301c);
            if (g_pPaletteTransitionDelta_005d3014 != 0)
                ReleasePacketHandle(g_pPaletteTransitionDelta_005d3014);
            if (g_pPaletteTransitionDirection_005d3010 != 0)
                ReleasePacketHandle(g_pPaletteTransitionDirection_005d3010);
            return 0;
        }

        g_nPaletteTransitionMaxDelta_005d3018 = 0;
        for (colour = 0; colour < componentCount; colour++) {
            difference = (short)(current[colour] - target[colour]);
            if (difference < 0) {
                difference = (short)-difference;
                g_pPaletteTransitionDirection_005d3010[colour] = 1;
            } else {
                g_pPaletteTransitionDirection_005d3010[colour] = -1;
            }
            g_pPaletteTransitionDelta_005d3014[colour] = difference;
            if (g_nPaletteTransitionMaxDelta_005d3018 < difference)
                g_nPaletteTransitionMaxDelta_005d3018 = difference;
        }

        difference = (short)(g_nPaletteTransitionMaxDelta_005d3018 / 2);
        for (colour = 0; colour < componentCount; colour++)
            g_pPaletteTransitionAccumulator_005d301c[colour] = difference;
        g_nPaletteTransitionCountdown_005d3020 =
            g_nPaletteTransitionMaxDelta_005d3018;
        g_nPaletteTransitionInitialise_0049305c = 0;
    }

    previousCountdown = g_nPaletteTransitionCountdown_005d3020;
    g_nPaletteTransitionCountdown_005d3020--;
    if (previousCountdown == 0) {
        ReleasePacketHandle(g_pPaletteTransitionAccumulator_005d301c);
        ReleasePacketHandle(g_pPaletteTransitionDelta_005d3014);
        ReleasePacketHandle(g_pPaletteTransitionDirection_005d3010);
        g_nPaletteTransitionInitialise_0049305c = 1;
        return 0;
    }

    for (colour = 0; colour < componentCount; colour++) {
        g_pPaletteTransitionAccumulator_005d301c[colour] =
            (short)(g_pPaletteTransitionAccumulator_005d301c[colour] +
                    g_pPaletteTransitionDelta_005d3014[colour]);
        if (g_pPaletteTransitionAccumulator_005d301c[colour] >
            g_nPaletteTransitionMaxDelta_005d3018) {
            g_pPaletteTransitionAccumulator_005d301c[colour] =
                (short)(g_pPaletteTransitionAccumulator_005d301c[colour] -
                        g_nPaletteTransitionMaxDelta_005d3018);
            current[colour] =
                (short)(current[colour] +
                        g_pPaletteTransitionDirection_005d3010[colour]);
        }
    }
    return 1;
}
