/*
 *  Number parser.
 *
 *  Retail object 0x453BD0-0x453C7F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x453BD0 */
/* Nothing in the shipped executable calls this.  Converts "0x...", "$..."
 * (hexadecimal) or decimal text of up to nine characters. */
short ParseNumber(char *text)
{
    char buffer[12];
    short value;

    if ((short)DosStrlen(text) > 9)
        return 0;
    if (toupper(text[1]) == 'X') {
        value = ParseHexNumber(text + 2);
    } else if (text[0] == '$') {
        value = ParseHexNumber(text + 1);
    } else {
        DosStrcpy(buffer, text);
        value = (short)atoi(buffer);
    }
    return value;
}
