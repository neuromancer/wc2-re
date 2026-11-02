/*
 *  Packet sections and data file handles.
 *
 *  Retail object 0x44F010-0x44F1EF (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x44F010 */
short OpenPacketSection(const char *filename, short section,
                        PacketSectionHandle *handle)
{
    unsigned int sectionEntry;
    unsigned int fileSize;
    unsigned int nextEntry;
    int sectionIndex;
    unsigned int directorySize;
    short compression;
    short sectionCount;
    short file;

    file = OpenDataFileOrDie(filename);
    if (file == -1)
        goto failed;
    if (ReadDataFileAtOffset(file, 0, 4, &fileSize) == 0)
        goto failed;
    if (ReadDataFileAtOffset(file, 4, 4, &directorySize) == 0)
        goto failed;
    sectionCount = (short)(directorySize >> 2) - 1;
    if (section >= sectionCount) {
        g_nPacketError_0049ca90 = 3;
        goto failed;
    }
    sectionIndex = (int)section;
    if (ReadDataFileAtOffset(file, sectionIndex * 4 + 4, 4,
                             &sectionEntry) == 0)
        goto failed;
    handle->finalSection = 0;
    compression = (short)(sectionEntry >> 24);
    if (sectionCount - sectionIndex == 1) {
        handle->finalSection = 1;
        nextEntry = fileSize;
        sectionEntry &= 0x00ffffff;
    } else {
        sectionEntry &= 0x00ffffff;
        if (ReadDataFileAtOffset(file, sectionIndex * 4 + 8, 4,
                                 &nextEntry) == 0)
            goto failed;
        nextEntry &= 0x00ffffff;
    }
    fileSize = nextEntry - sectionEntry;
    if (SeekDataFile(file, sectionEntry, 0) != 0) {
        handle->file = file;
        handle->dataOffset = sectionEntry;
        handle->sectionCount = sectionCount;
        handle->compression = compression;
        handle->dataSize = fileSize;
        handle->position = 0;
        return 1;
    }
failed:
    CloseDataFile((unsigned short)file);
    return 0;
}

/* Function start: 0x44F1C7 */
void CloseDataFileByHandle(unsigned short *handle)
{
    CloseDataFile(*handle);
}
