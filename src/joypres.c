/*
 *  Joystick presence query.
 *
 *  Retail object 0x401820-0x40183F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x401820 */
/* Nothing in the shipped executable calls this.  Always reports a
 * joystick. */
unsigned short GetJoystickPresentUnused(void)
{
    return 1;
}
