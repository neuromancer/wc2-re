/*
 *  Packet size.
 *
 *  Retail object 0x453A70-0x453B2F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x453A70 */
unsigned int GetPacketSize(const char *filename, short section)
{
    PacketSectionHandle handle;
    unsigned int size;

    size = (unsigned int)-1;
    if (OpenPacketSection(filename, section, &handle) != 0) {
        switch (handle.compression) {
        case 1:
            ReadPacketSectionData(&handle, &size, 4);
            if (g_nPacketError != 0)
                size = (unsigned int)-1;
            break;
        case 2:
            size = handle.dataSize;
            break;
        default:
            size = handle.dataSize;
            break;
        }
        CloseDataFileByHandle((unsigned short *)&handle);
    }
    return size;
}
