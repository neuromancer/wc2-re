/*
 *  Sets a viewport rectangle.
 *
 *  Retail object 0x423C60-0x423C9F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x423C60 */
void SetViewportRect(Viewport *viewport, unsigned short left,
                     unsigned short top, unsigned short right,
                     unsigned short bottom)
{
    viewport->left = (short)left;
    viewport->top = (short)top;
    viewport->right = (short)right;
    viewport->bottom = (short)bottom;
}
