/*
 *  String concatenation shim.
 *
 *  Retail object 0x446910-0x44694F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x446910 */
char *DosStrcat(char *destination, const char *source)
{
    char *end;

    end = DosStrchr(destination, 0);
    DosStrcpy(end, source);
    return destination;
}
