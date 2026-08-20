/*
 *  Packet loader.
 *
 *  Address range 0x42b000-0x42b3ff (provisional -- see docs/ORDER.md).
 *  Boundary evidence: PROVEN: PacketLoad prints "Library\\Source\\Pload.c PacketLoad".
 */
#include "wc1.h"

#pragma function(strcmp)

/* Function start: 0x446560 */
short LoadGraphicsDriver(short rewritePacketExtensions)
{
    (void)rewritePacketExtensions;
    return 1;
}

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
        case 1:
#ifdef WC1_SDL
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
                g_nPacketError_0049ca90 = 6;
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
                g_nPacketError_0049ca90 = 1;
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
            g_pLastPacketAllocation_005c80e0 = data;
            if (data == 0) {
                g_nPacketError_0049ca90 = 4;
            } else if (!Wc1SdlDecompressOriginLzw(
                           compressedData, compressedSize, data,
                           outputSize, &writtenSize)) {
                if (allocatedPacket != 0)
                    ReleasePacketHandle(data);
                data = 0;
                g_pLastPacketAllocation_005c80e0 = 0;
                g_nPacketError_0049ca90 = 6;
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
                g_nPacketError_0049ca90 = 8;
            } else {
                data = destination;
                if (data == 0) {
                    g_pLastPacketAllocation_005c80e0 = AllocateTaggedMemory(
                        handle.dataSize, (short)(flags | 0x40));
                    data = g_pLastPacketAllocation_005c80e0;
                    if (data == 0)
                        g_nPacketError_0049ca90 = 4;
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
            }
            break;
        }
        CloseDataFileByHandle((unsigned short *)&handle);
    }
    return data;
}

/* Function start: 0x423CA0 */
void InitializeAudioSystem(HWND window)
{
    if (g_nAudioEnabled_0049c244 != 0 && g_bAudioSystemInitialized_004961b0 == 0) {
        ix_system_configure(3, (void *)1);
        ix_system_configure(0, window);
        ix_system_init();
        ix_system_set_voice_count(0x10);
        g_bAudioSystemInitialized_004961b0 = 1;
    }
}

/* Function start: 0x423D02 */
void ServiceAudioStream(void)
{
    if (g_nAudioEnabled_0049c244 != 0) {
        if (g_bAudioSystemInitialized_004961b0 != 0) {
            ix_system_delete_all_sounds();
            ix_system_delete_all_samples();
            ix_system_shutdown();
            FreeWaveTable();
            g_bAudioSystemInitialized_004961b0 = 0;
        }
        return;
    }
}

/* Function start: 0x423D4F */
WaveTableEntry *AllocateWaveTableEntry(void)
{
    if (g_pWaveTableHead_004961b4 == 0) {
        g_pWaveTableHead_004961b4 =
            malloc(sizeof(WaveTableEntry));
        g_pWaveTableTail_004961b8 = g_pWaveTableHead_004961b4;
    } else {
        g_pWaveTableTail_004961b8->next =
            malloc(sizeof(WaveTableEntry));
        g_pWaveTableTail_004961b8 = g_pWaveTableTail_004961b8->next;
    }
    g_pWaveTableTail_004961b8->next = 0;
    return g_pWaveTableTail_004961b8;
}

/* Function start: 0x423DBB */
WaveTableEntry *FindWaveTableEntryByName(const char *name)
{
    WaveTableEntry *entry = g_pWaveTableHead_004961b4;

    while (entry != 0) {
        if (strcmp(entry->name, name) == 0)
            return entry;
        entry = entry->next;
    }
    return 0;
}

/* Function start: 0x423E12 */
void RemoveWaveTableEntry(WaveTableEntry *target)
{
    WaveTableEntry *previous = 0;
    WaveTableEntry *entry = g_pWaveTableHead_004961b4;

    while (entry != 0 && target != entry) {
        previous = entry;
        entry = entry->next;
    }
    if (entry != 0) {
        if (previous != 0)
            previous->next = entry->next;
        if (g_pWaveTableTail_004961b8 == entry && previous != 0)
            g_pWaveTableTail_004961b8 = previous;
        if (g_pWaveTableHead_004961b4 == entry)
            g_pWaveTableHead_004961b4 = entry->next;
        free(entry->name);
        free(entry);
    }
}

/* Function start: 0x423ED1 */
void FreeWaveTable(void)
{
    WaveTableEntry *entry;
    WaveTableEntry *next;

    entry = g_pWaveTableHead_004961b4;
    next = 0;
    while (entry != 0) {
        next = entry->next;
        free(entry->name);
        free(entry);
        entry = next;
    }
    g_pWaveTableHead_004961b4 = g_pWaveTableTail_004961b8 = 0;
    return;
}

/* Function start: 0x423F3F */
ActiveSoundEntry *AllocateActiveSoundEntry(void)
{
    if (g_pActiveSoundHead_004961a8 == 0) {
        g_pActiveSoundHead_004961a8 =
            malloc(sizeof(ActiveSoundEntry));
        g_pActiveSoundTail_004961ac = g_pActiveSoundHead_004961a8;
    } else {
        g_pActiveSoundTail_004961ac->next =
            malloc(sizeof(ActiveSoundEntry));
        g_pActiveSoundTail_004961ac = g_pActiveSoundTail_004961ac->next;
    }
    g_pActiveSoundTail_004961ac->next = 0;
    return g_pActiveSoundTail_004961ac;
}

/* Function start: 0x423FAB */
void RemoveActiveSoundEntry(ActiveSoundEntry *target)
{
    ActiveSoundEntry *entry = g_pActiveSoundHead_004961a8;
    ActiveSoundEntry *previous = 0;

    while (entry != 0 && entry != target) {
        previous = entry;
        entry = entry->next;
    }
    if (entry != 0) {
        if (previous != 0)
            previous->next = entry->next;
        if (g_pActiveSoundTail_004961ac == entry && previous != 0)
            g_pActiveSoundTail_004961ac = previous;
        if (g_pActiveSoundHead_004961a8 == entry)
            g_pActiveSoundHead_004961a8 = entry->next;
        free(entry);
    }
}

/* Function start: 0x42405E */
ActiveSoundEntry *FindActiveSoundEntryBySample(IxSample *sample)
{
    ActiveSoundEntry *entry;

    entry = g_pActiveSoundHead_004961a8;
    while (entry != 0) {
        if (ix_sound_get_sample(entry->sound) == sample)
            return entry;
        entry = entry->next;
    }
    return 0;
}
