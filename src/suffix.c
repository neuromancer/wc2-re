/*
 *  Graphics file suffixes and character conversion.
 *
 *  Retail object 0x42BAC0-0x42BB6F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x42BAC0 */
void FillGraphicSuffix(char *path, short number, short digits)
{
    char *dot;
    char *extension;

    extension = DosStrchr(path, '.');
    dot = extension;
    extension++;
    if (dot == 0)
        extension = path;
    extension[digits] = '\0';
    ConvertChar_Int(extension, number, digits);
}

/* Function start: 0x42BB17 */
void ConvertChar_Int(char *text, short number, short digits)
{
    while (digits--) {
        text[digits] = (char)(number % 10) + '0';
        number /= 10;
    }
}
