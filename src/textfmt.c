/*
 *  Formatted text output.
 *
 *  Retail object 0x420340-0x4207AF (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x420340 */
void EmitTextString(void (*writer)(int), const char *text)
{
    short character;

    while ((character = (signed char)*text++) != 0)
        writer((short)character);
}

/* Function start: 0x420378 */
void FormatTextTokens(void (*writer)(int),
#ifdef SDL_PORT
                      const char *format, va_list *arguments)
#else
                      const char *format, va_list arguments)
#endif
{
    short spec;
    char number[64];
    short ch;

    if (g_pCurrentTextContext_005c8d1c == 0)
        return;
    while ((ch = (signed char)*format++) != 0) {
        switch (ch) {
        case '%':
            spec = (signed char)*format++;
            switch (spec) {
                case 'd':
#ifdef SDL_PORT
                    EmitTextString(
                        writer,
                        _itoa((short)va_arg(*arguments, int), number, 10));
#else
                    EmitTextString(writer,
                                   _itoa(va_arg(arguments, short), number, 10));
#endif
                    break;
                case 'u':
#ifdef SDL_PORT
                    EmitTextString(
                        writer,
                        _ultoa((unsigned short)va_arg(*arguments, unsigned int),
                               number, 10));
#else
                    EmitTextString(
                        writer,
                        _ultoa(va_arg(arguments, unsigned short), number, 10));
#endif
                    break;
                case 'D':
#ifdef SDL_PORT
                    EmitTextString(writer,
                                   _ltoa(va_arg(*arguments, long), number, 10));
#else
                    EmitTextString(writer,
                                   _ltoa(va_arg(arguments, long), number, 10));
#endif
                    break;
                case 'U':
#ifdef SDL_PORT
                    EmitTextString(
                        writer,
                        _ultoa(va_arg(*arguments, unsigned long), number, 10));
#else
                    EmitTextString(
                        writer,
                        _ultoa(va_arg(arguments, unsigned long), number, 10));
#endif
                    break;
                case 'x':
#ifdef SDL_PORT
                    EmitTextString(
                        writer,
                        _strupr(_ultoa(
                            (unsigned short)va_arg(*arguments, unsigned int),
                            number, 16)));
#else
                    EmitTextString(
                        writer,
                        _strupr(_ultoa(va_arg(arguments, unsigned short),
                                       number, 16)));
#endif
                    break;
                case 'c':
#ifdef SDL_PORT
                    writer((short)va_arg(*arguments, int));
#else
                    writer(va_arg(arguments, short));
#endif
                    break;
                case 'S':
                case 's':
#ifdef SDL_PORT
                    EmitTextString(writer, va_arg(*arguments, char *));
#else
                    EmitTextString(writer, va_arg(arguments, char *));
#endif
                    break;
                case 'X':
#ifdef SDL_PORT
                    g_pCurrentTextContext_005c8d1c->cursorX =
                        (short)va_arg(*arguments, int);
#else
                    g_pCurrentTextContext_005c8d1c->cursorX =
                        va_arg(arguments, short);
#endif
                    break;
                case 'Y':
#ifdef SDL_PORT
                    g_pCurrentTextContext_005c8d1c->cursorY =
                        (short)va_arg(*arguments, int);
#else
                    g_pCurrentTextContext_005c8d1c->cursorY =
                        va_arg(arguments, short);
#endif
                    break;
                case 'B':
#ifdef SDL_PORT
                    g_pCurrentTextContext_005c8d1c->backgroundColour =
                        (unsigned char)va_arg(*arguments, int);
#else
                    g_pCurrentTextContext_005c8d1c->backgroundColour =
                        va_arg(arguments, unsigned char);
#endif
                    break;
                case 'F':
#ifdef SDL_PORT
                    g_pCurrentTextContext_005c8d1c->colour =
                        (unsigned char)va_arg(*arguments, int);
#else
                    g_pCurrentTextContext_005c8d1c->colour =
                        va_arg(arguments, unsigned char);
#endif
                    break;
                case 'P':
                    if (g_bCinematicSpriteFontEnabled_005c82a7 == 0)
                        DrawTextString(g_pCurrentTextContext_005c8d1c->text);
                    else
                        DrawWrappedCinematicText(
                            g_pCurrentTextContext_005c8d1c->text);
                    break;
                case 'J':
#ifdef SDL_PORT
                    g_pCurrentTextContext_005c8d1c->alignment =
                        (unsigned char)va_arg(*arguments, int);
#else
                    g_pCurrentTextContext_005c8d1c->alignment =
                        va_arg(arguments, unsigned char);
#endif
                    break;
                default:
                    writer(spec);
                    break;
            }
            break;
        default:
            writer(ch);
            break;
        }
    }
}

/* Function start: 0x42067F */
void DrawFormattedText(const char *format, ...)
{
    va_list arguments;

    va_start(arguments, format);
    if (g_bCinematicSpriteFontEnabled_005c82a7 == 0) {
        FormatTextTokens((void (*)(int))DrawTextCharacter, format,
#ifdef SDL_PORT
                         &arguments);
#else
                         arguments);
#endif
    } else {
        FormatTextTokens((void (*)(int))DrawCinematicFontCharacter,
                         format,
#ifdef SDL_PORT
                         &arguments);
#else
                         arguments);
#endif
    }
    va_end(arguments);
    if (g_pCurrentTextContext_005c8d1c->viewport->pixels ==
        g_stScreenViewport_005d21a0.pixels)
        MarkDibDirty();
}

/* Function start: 0x4206F2 */
void FormatTextBufferFromStart(const char *format, ...)
{
    va_list arguments;

    va_start(arguments, format);
    if (g_pCurrentTextContext_005c8d1c == 0)
        return;
    g_pCurrentTextContext_005c8d1c->textCursor =
        g_pCurrentTextContext_005c8d1c->text;
    FormatTextTokens((void (*)(int))AppendTextCharacter, format,
#ifdef SDL_PORT
                     &arguments);
#else
                     arguments);
#endif
    va_end(arguments);
    if (g_pCurrentTextContext_005c8d1c->viewport->pixels ==
        g_stScreenViewport_005d21a0.pixels)
        MarkDibDirty();
}

/* Function start: 0x42075F */
void AppendFormattedText(const char *format, ...)
{
    va_list arguments;

    va_start(arguments, format);
    FormatTextTokens((void (*)(int))AppendTextCharacter, format,
#ifdef SDL_PORT
                     &arguments);
#else
                     arguments);
#endif
    va_end(arguments);
    if (g_pCurrentTextContext_005c8d1c->viewport->pixels ==
        g_stScreenViewport_005d21a0.pixels)
        MarkDibDirty();
}
