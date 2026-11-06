/*
 *  Reads packet section data.
 *
 *  Retail object 0x469BE0-0x469C7F (link order; see docs/ORDER.md).
 */
#include "game.h"
#ifndef SDL_PORT
#include <malloc.h>
#endif

/* Function start: 0x469BE0 */
short ReadPacketSectionData(PacketSectionHandle *handle,
                            void *destination,
                            unsigned int length)
{
    int offset;
    int end;

    offset = (int)(handle->dataOffset + handle->position);
    end = (int)(handle->dataOffset + handle->dataSize);
    if (destination == 0)
        return 0;
    if (end < (int)(offset + length) || length == (unsigned int)-1)
        length = (unsigned int)(end - offset);
    if (ReadDataFileAtOffset((unsigned short)handle->file, offset,
                             length, destination) == 0)
        return 0;
    handle->position += length;
    return 1;
}
