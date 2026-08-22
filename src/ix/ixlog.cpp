/*
 *  ix diagnostic printer.
 *
 *  MODULE BOUNDARY UNVERIFIED: ix_log_printf sits at 0x00428BD4, separate
 *  from the other mapped ix ranges, so its original object extent is not yet
 *  known. Built /Od like the rest of ix.
 *
 *  Every ix diagnostic goes through here, 107 call sites, always in pairs:
 *      ix_log_printf("Fatal [%s - %d]:\n", __FILE__, __LINE__);
 *      ix_log_printf("human readable message");
 */
#include "ix.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#ifndef SDL_PORT
#include <crtdbg.h>
#pragma function(strcpy)
#endif


/* Formatting scratch buffer. */
char g_szIxLogBuffer_005b2c98[1024];

/* Mono-monitor debug printer in the game core (C linkage). */
extern "C" void SoundDebugPrintf(const char *fmt, ...);   /* 0x00437946 */

/* Function start: 0x428BD4 */
void ix_log_printf(const char *fmt, ...)
{
    va_list arguments;

    if (fmt != 0) {
        va_start(arguments, fmt);
        vsprintf(g_szIxLogBuffer_005b2c98, fmt, arguments);
        va_end(arguments);
    } else {
        strcpy(g_szIxLogBuffer_005b2c98, "(null)");
    }
#ifdef SDL_PORT
    SdlOutputDebugString(g_szIxLogBuffer_005b2c98);
#else
    OutputDebugStringA(g_szIxLogBuffer_005b2c98);
#endif
}
