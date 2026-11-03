/*
 *  Packet sections and data file handles.
 *
 *  Retail object 0x44F010-0x44F1EF (link order; see docs/ORDER.md).
 */
#include "game.h"

short OpenDataFileOrDie(const char *path);
int ReadDataFileAtOffset(unsigned short fd, int offset, unsigned int length,
                         void *data);
int SeekDataFile(unsigned short fd, int offset, unsigned int origin);

/* Function start: 0x44F010 */
short OpenPacketSection(const char *filename, short section,
                        PacketSectionHandle *out)
{
    unsigned int start;
    unsigned int length;
    unsigned int next;
    int offset;
    unsigned int directorySize;
    short method;
    short entryCount;
    short handle;

    handle = OpenDataFileOrDie(filename);
    if (handle != -1) {
        for (;;) {
            if (ReadDataFileAtOffset(handle, 0, 4, &length) == 0)
                goto failed;
            if (ReadDataFileAtOffset(handle, 4, 4, &directorySize) == 0)
                goto failed;
            entryCount = (short)((directorySize >> 2) - 1);
            if (section >= entryCount) {
                g_nPacketError_0049ca90 = 3;
                goto failed;
            }
            offset = (section + 1) * 4;
            if (ReadDataFileAtOffset(handle, offset, 4, &start) == 0)
                goto failed;
            method = (short)(start >> 24);
            start &= 0x00ffffff;
            out->finalSection = 0;
            if (entryCount - 1 == section) {
                next = length;
                out->finalSection = 1;
            } else {
                offset += 4;
                if (ReadDataFileAtOffset(handle, offset, 4, &next) == 0)
                    goto failed;
                /* The top byte holds the next section's method flag. */
                ((unsigned char *)&next)[3] = 0;
            }
            length = next - start;
            if (SeekDataFile(handle, start, 0) == 0)
                goto failed;
            out->file = handle;
            out->sectionCount = entryCount;
            out->compression = method;
            out->dataOffset = start;
            out->position = 0;
            out->dataSize = length;
            return 1;
        }
    }
failed:
    CloseDataFile((unsigned short)handle);
    return 0;
}

/* Function start: 0x44F1C7 */
void CloseDataFileByHandle(unsigned short *handle)
{
    CloseDataFile(*handle);
}
