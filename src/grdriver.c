/*
 *  Loads the graphics driver.
 *
 *  Retail object 0x446560-0x44657F (link order; see docs/ORDER.md).
 */
#include "game.h"

#pragma function(strcmp)
/* Function start: 0x446560 */
short LoadGraphicsDriver(short rewritePacketExtensions)
{
    (void)rewritePacketExtensions;
    return 1;
}
