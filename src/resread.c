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

#ifdef SDL_PORT
    if (handle->patchedData != 0) {
        if (destination == 0 || handle->position > handle->dataSize)
            return 0;
        if (length > handle->dataSize - handle->position)
            length = handle->dataSize - handle->position;
        memcpy(destination, handle->patchedData + handle->position, length);
        handle->position += length;
        g_nPacketError = 0;
        return 1;
    }
#endif
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
