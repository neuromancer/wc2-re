/*
 *  Hexadecimal number parser.
 *
 *  Retail object 0x452930-0x4529FF (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Digit values by position; a space reads as 0. */
static char g_szHexDigits[] = "0123456789ABCDEF "; /* 0x0049be70 */

/* Function start: 0x452930 */
/* Converts up to four hexadecimal digits; anything else gives 0.  A space
 * counts as 0. */
short ParseHexNumber(char *text)
{
    short length;
    short value;
    short position;
    char *match;

    length = DosStrlen(text);
    value = 0;
    if (length > 4)
        return 0;
    for (position = length - 1; position > -1; position--) {
        value <<= 4;
        match = DosStrchr(g_szHexDigits, toupper(text[position]));
        if (match == 0)
            return 0;
        if (*match == ' ')
            match = g_szHexDigits;
        value += match - g_szHexDigits;
    }
    return value;
}
