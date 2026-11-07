/*
 *  Clamped text pixel width.
 *
 *  Retail object 0x453B30-0x453BCF (link order; see docs/ORDER.md).
 */
#include "game.h"

#pragma function(memset, strcat)
/* Function start: 0x453B30 */
short MeasureTextPixelWidthClamped(const char *text)
{
    const char *scan;
    short total;

    total = 0;
    scan = text;
    while (total < 320 && *scan != 0)
        total = (short)(total + GetFontCharWidth(*scan++));
    if (*scan-- != 0)
        total = (short)(total - GetFontCharWidth(*scan));
    return total;
}
