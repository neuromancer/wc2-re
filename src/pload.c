/*
 *  Packet loader (Library\Source\Pload.c).
 *
 *  Retail object 0x446580-0x44670F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x446580 */
void ResetStringBuilder(TextContext *context)
{
    context->textCursor = context->text;
    *context->textCursor = 0;
}

/* Function start: 0x4465A0 */
void *PacketLoad(const char *filename, short section,
                 void *destination, short flags,
                 void *decompressionWorkspace,
                 int registerHandle)
{
    unsigned char *data;
    PacketSectionHandle handle;

    data = 0;
    if (OpenPacketSection(filename, section, &handle) != 0) {
        switch (handle.compression) {
#ifdef SDL_PORT
        /* A DOS install marks its compressed sections 0x20 where the Saga
         * conversion decompressed them and marked them 0xe0.  The payload is
         * the same shape as compression 1 - a four byte length then the LZW
         * stream - and only thirteen of its files still carry any, brief.pal
         * among them. */
        case 0x20:
#endif
        case 1:
#ifdef SDL_PORT
        {
            unsigned char sizeBytes[4];
            unsigned char *compressedData;
            unsigned int compressedSize;
            unsigned int outputSize;
            size_t writtenSize;
            int allocatedPacket;

            if (handle.dataSize < 4 ||
                ReadDataFileAtOffset((unsigned short)handle.file,
                                     (int)handle.dataOffset, 4,
                                     sizeBytes) == 0) {
                g_nPacketError = 6;
                break;
            }
            outputSize = (unsigned int)sizeBytes[0] |
                ((unsigned int)sizeBytes[1] << 8) |
                ((unsigned int)sizeBytes[2] << 16) |
                ((unsigned int)sizeBytes[3] << 24);
            compressedSize = handle.dataSize - 4;
            compressedData = (unsigned char *)malloc(
                compressedSize != 0 ? compressedSize : 1);
            if (compressedData == 0) {
                g_nPacketError = 1;
                break;
            }
            handle.position = 4;
            if (ReadPacketSectionData(&handle, compressedData,
                                      compressedSize) == 0) {
                free(compressedData);
                data = 0;
                break;
            }

            allocatedPacket = destination == 0;
            data = destination;
            if (data == 0)
                data = AllocateTaggedMemory(outputSize, flags);
            g_26_pLibraryCurrentLastPacketAllocation = data;
            if (data == 0) {
                g_nPacketError = 4;
            } else if (!SdlDecompressOriginLzw(
                           compressedData, compressedSize, data,
                           outputSize, &writtenSize)) {
                if (allocatedPacket != 0)
                    ReleasePacketHandle(data);
                data = 0;
                g_26_pLibraryCurrentLastPacketAllocation = 0;
                g_nPacketError = 6;
            }
            free(compressedData);
            break;
        }
#else
            SystemDebugPrintf(
                "[SYSTEM] : ERROR\n"
                "Library\\Source\\Pload.c PacketLoad  Compressed data in '%s'\n",
                filename);
            SystemDebugPrintf("[SYSTEM] : Exiting prematurely\n");
            ClearDebugPauseFlags();
            PumpMessagesDuringWait();
            exit(0);
            break;
#endif
        default:
            if (handle.dataSize == 0) {
                data = 0;
                g_nPacketError = 8;
                break;
            }
            data = destination;
            if (data == 0) {
                g_26_pLibraryCurrentLastPacketAllocation = AllocateTaggedMemory(
                    handle.dataSize, (short)(flags | 0x40));
                data = g_26_pLibraryCurrentLastPacketAllocation;
                if (data == 0)
                    g_nPacketError = 4;
            }
            if (data != 0) {
                if (registerHandle != 0 &&
                    IsPushedPacketHandle(data) == 0)
                    exit_squadron(
                        "qq PacketLoad with non-pushed dest");
                if (ReadPacketSectionData(
                        &handle, data, handle.dataSize) == 0)
                    data = 0;
            }
            break;
        }
        CloseDataFileByHandle((unsigned short *)&handle);
    }
    return data;
}
