/*
 *  16-bit DOS C string and memory shims.
 *
 *  Address range 0x435400-0x4355ff (provisional -- see docs/ORDER.md).
 *  Boundary evidence: all __stdcall with short-width arguments, each forwarding to one CRT routine.
 */
#include "wc1.h"

#pragma function(strlen, strcpy, memcpy, memset)

/* Function start: 0x46243F */
unsigned int DosFarPtrToNear(void *v)
{
    return (unsigned int)v;
}

/* Function start: 0x462452 */
void *DosNearPtrToFar(unsigned int v)
{
    return (void *)v;
}

/* Function start: 0x462465 */
char *DosStrrchr(char *s, short c)
{
    return strrchr(s, (int)c);
}

/* Function start: 0x462486 */
char *DosStrchr(const char *s, short c)
{
    return strchr(s, (int)c);
}

/* Function start: 0x4624A7 */
char *DosStrcpy(char *dst, const char *src)
{
    return strcpy(dst, src);
}

/* Function start: 0x4624C7 */
char *CopyStringAndReturnEnd(char *destination, const char *source)
{
    strcpy(destination, source);
    return destination + strlen(destination);
}

/* Function start: 0x46250B */
short DosStrlen(const char *s)
{
    return (short)strlen(s);
}

/* Function start: 0x462527 */
void DosMemcpy(void *dst, const void *src, size_t n)
{
    memcpy(dst, src, n & 0xffff);
}

/* Function start: 0x462550 */
void DosMemset(void *destination, unsigned int count, short value)
{
    memset(destination, value, (unsigned short)count);
}

/* Function start: 0x46257A */
short InitializePostPaletteState(void)
{
    return 0;
}

/* Function start: 0x46259D */
short InitializeInputManagerMemory(void)
{
    g_bInputManagerInitialized_005c80ca = 1;
    g_pInputManagerState_005c8464 =
        (InputManagerState *)AllocateTaggedMemory(0x21, 0x40);
    return 1;
}

/* Function start: WC2_UNMAPPED */
unsigned short GetEventManagerStatus(void)
{
    return 0;
}

/* Function start: WC2_UNMAPPED */
void __stdcall RegisterEventManagerShutdown(void (*fn)(void))
{
}

/* Function start: WC2_UNMAPPED */
short __stdcall InitializeEventManager(short period,
                                       unsigned short (*initialize)(void),
                                       void *configuration)
{
    g_bInputManagerInitialized_005c80ca = 1;
    if (initialize != 0)
        initialize();
    return 1;
}

/* Function start: WC2_UNMAPPED */
void ShutdownEventManager(void)
{
    g_bInputManagerInitialized_005c80ca = 0;
}

/* Function start: WC2_UNMAPPED */
unsigned short __stdcall ConfigureEventManagerPointer(
    unsigned char *shape, short frame)
{
    return 0;
}

/* Function start: WC2_UNMAPPED */
void __stdcall SetEventManagerPump(void (*pump)(void))
{
    DAT_0059ab2c = pump;
}
