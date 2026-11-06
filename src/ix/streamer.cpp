/*
 *  ix / D:\Rnd\Prj\ix\Src\Streamer\streamer.cpp
 *
 *  Retail address range: 0x0046BD50 - 0x0046D10F (23 functions)
 *  Object boundaries follow retail padding and alignment; diagnostic paths
 *  identify the source subsystem (see docs/ORDER.md).
 *
 *  Implement in ADDRESS ORDER -- MSVC emits functions in source order, so the
 *  list below is the original source order of this file.
 */
#include "ix.h"
#include <stdlib.h>

#define IX_STREAMER_FILE "D:\\Rnd\\Prj\\ix\\Src\\Streamer\\streamer.cpp"

unsigned int *g_7_pStreamerPacketOffsets; /* 0x005c4b38 */
unsigned int g_18_dwStreamerThreadTick; /* 0x005c4b3c */
FILE *g_1_pFeederPackageDisk; /* 0x005c4b44 */
IxStreamFile *g_48_pPumpIdleFiles; /* 0x005c4b48 */
unsigned int g_13_adwPlayerBranchingStack[32]; /* 0x005c4b50 */
unsigned char *g_126_pReaderCompressedBuf; /* 0x005c4bd0 */
IxStreamFile *g_1_pPlayerReadQueue; /* 0x005c4bd4 */
unsigned int g_49_nStreamerAudioBufferSize; /* 0x005c4bd8 */
unsigned char *g_pStreamerFileStore; /* 0x005c4bdc */
unsigned char g_6_bFeederLevel; /* 0x005c4be0 */
HANDLE g_4_hStreamerWorker; /* 0x005c4be4 */
IxStreamerFileEntry *g_3_pReaderFileEntries; /* 0x005c4be8 */
IxStreamerHeader *g_pStreamPrologue; /* 0x005c4bec */
IxStreamerAudioChunk *g_15_pStreamerSoundChunks; /* 0x005c4bf0 */
IxStreamerBranch *g_7_pStreamBranches; /* 0x005c4bf4 */
IxStreamerFileChunk *g_17_pStreamerFileParts; /* 0x005c4bf8 */
CRITICAL_SECTION g_csStreamerFileFifo; /* 0x005c4c00 */
CRITICAL_SECTION g_49_csStreamWorker; /* 0x005c4c18 */
unsigned int g_6_dwStreamSoundChunk; /* 0x005c4c30 */
DWORD g_dwStreamThreadId; /* 0x005c4c34 */
unsigned int g_dwStreamerMode; /* 0x005c4c38 */
HANDLE g_1_hStreamRouseEvent; /* 0x005c4c3c */
unsigned int g_nSpoolerDiskChunk; /* 0x005c4c40 */
unsigned int g_16_nStreamerBytesEachSecond; /* 0x005c4c44 */
CRITICAL_SECTION g_1_csStreamer; /* 0x005c4c48 */
IxStreamerTrigger *g_6_pFeederTriggers; /* 0x005c4c60 */

unsigned short g_nStreamerVolume = 0xffff; /* 0x0049e150 */
char g_cStreamerBranchTag = -1; /* 0x0049e154 */
unsigned int g_nStreamerBranchStackIndex = 0; /* 0x0049e158 */

/* Function start: 0x46BD50 */   /* source line(s) 60;63;75: Streamer already inited! | Failed to init DSP | Failed to start streamer_thread! */
extern "C" int ix_streamer_init(void)
{
    if ((g_dwStreamerMode & IX_STREAMER_INITIALIZED) != 0) {
        ix_log_printf("Warning [%s - %d]:\n", IX_STREAMER_FILE, 60);
        ix_log_printf("Streamer already inited!");
        return -1;
    }
    if (ix_dsp_init() != 0) {
        ix_log_printf("Warning [%s - %d]:\n", IX_STREAMER_FILE, 63);
        ix_log_printf("Failed to init DSP");
        return -1;
    }
    InitializeCriticalSection(&g_csStreamerFileFifo);
    InitializeCriticalSection(&g_1_csStreamer);
    InitializeCriticalSection(&g_49_csStreamWorker);
    g_1_hStreamRouseEvent = CreateEventA(0, TRUE, FALSE, 0);
    g_4_hStreamerWorker =
        CreateThread(0, 0x1000, ix_streamer_thread_proc, 0, 0,
                     &g_dwStreamThreadId);
    if (g_4_hStreamerWorker == 0) {
        ix_log_printf("Fatal [%s - %d]:\n", IX_STREAMER_FILE, 75);
        ix_log_printf("Failed to start streamer_thread!");
        exit(-1);
    }
    g_dwStreamerMode |= IX_STREAMER_INITIALIZED;
    g_dwStreamerMode |= IX_STREAMER_DEVELOPER_MODE;
    return 0;
}

/* Function start: 0x46BE6F */   /* source line(s) 87: Streamer not ready for destroy */
#ifdef SDL_PORT
extern "C"
#endif
extern "C" void ix_streamer_destroy(void)
{
    if ((g_dwStreamerMode & IX_STREAMER_INITIALIZED) == 0) {
        ix_log_printf("Warning [%s - %d]:\n", IX_STREAMER_FILE, 87);
        ix_log_printf("Streamer not ready for destroy");
        return;
    }
    if ((g_dwStreamerMode & IX_STREAMER_FILE_OPEN) != 0)
        ix_streamer_close_stream_file();
    g_dwStreamerMode |= IX_STREAMER_SHUTDOWN;
    SetEvent(g_1_hStreamRouseEvent);
#ifdef SDL_PORT
    WaitForSingleObject(g_4_hStreamerWorker, INFINITE);
    CloseHandle(g_4_hStreamerWorker);
    g_4_hStreamerWorker = 0;
#else
    EnterCriticalSection(&g_49_csStreamWorker);
#endif
    CloseHandle(g_1_hStreamRouseEvent);
    DeleteCriticalSection(&g_csStreamerFileFifo);
    DeleteCriticalSection(&g_1_csStreamer);
    DeleteCriticalSection(&g_49_csStreamWorker);
    ix_dsp_shutdown();
    g_dwStreamerMode &= ~IX_STREAMER_INITIALIZED;
#ifdef SDL_PORT
    g_dwStreamerMode &=
        ~(IX_STREAMER_SHUTDOWN | IX_STREAMER_THREAD_RUNNING);
#endif
}

/* Function start: 0x46BF1E */
extern "C" void ix_streamer_configure(int option, void *value)
{
    ix_dsp_configure(option, value);
}

/* Function start: 0x46BF3E */   /* source line(s) 117: Can't change dev_mode while stream files are open! */
void ix_streamer_set_dev_mode(int mode)
{
    if (g_48_pPumpIdleFiles != 0 ||
        g_1_pPlayerReadQueue != 0) {
        ix_log_printf("Fatal [%s - %d]:\n", IX_STREAMER_FILE, 117);
        ix_log_printf("Can't change dev_mode while stream files are open!");
        exit(-1);
    }
    if (mode != 0)
        g_dwStreamerMode |= IX_STREAMER_DEVELOPER_MODE;
    else
        g_dwStreamerMode &= ~IX_STREAMER_DEVELOPER_MODE;
}

/* Function start: 0x46BFB6 */   /* source line(s) 129;142;146;150;151;157;168;177;186;195;204;238;240: Streamer not ready to open stream file! | Failed to open stream file. | Streamer failed to */
extern "C" int ix_streamer_open_stream_file(char *path)
{
    if ((g_dwStreamerMode & IX_STREAMER_INITIALIZED) == 0) {
        ix_log_printf("Warning [%s - %d]:\n", IX_STREAMER_FILE, 129);
        ix_log_printf("Streamer not ready to open stream file!");
        return -1;
    }
    if ((g_dwStreamerMode & IX_STREAMER_FILE_OPEN) != 0)
        ix_streamer_close_stream_file();
    g_48_pPumpIdleFiles = 0;
    g_6_dwStreamSoundChunk = 0;
    g_nSpoolerDiskChunk = (unsigned int)-1;
    g_nStreamerBranchStackIndex = 0;
    g_1_pFeederPackageDisk = ix_file_open(path, 1);
    if (g_1_pFeederPackageDisk == 0) {
        ix_log_printf("Warning [%s - %d]:\n", IX_STREAMER_FILE, 142);
        ix_log_printf("Failed to open stream file.");
        return -1;
    }
    g_pStreamPrologue = (IxStreamerHeader *)
        g_pIxMalloc(sizeof(IxStreamerHeader));
    if (g_pStreamPrologue == 0) {
        ix_log_printf("Fatal [%s - %d]:\n", IX_STREAMER_FILE, 146);
        ix_log_printf("Streamer failed to allocate.");
        exit(-1);
    }
    ix_file_read(g_1_pFeederPackageDisk,
                 g_pStreamPrologue, sizeof(IxStreamerHeader));
    if (g_pStreamPrologue->id != *(unsigned int *)"STRM") {
        ix_log_printf("Warning [%s - %d]:\n", IX_STREAMER_FILE, 150);
        ix_log_printf("Invalid stream ID");
        return -1;
    }
    if (g_pStreamPrologue->version != 1) {
        ix_log_printf("Warning [%s - %d]:\n", IX_STREAMER_FILE, 151);
        ix_log_printf("Invalid stream version");
        return -1;
    }
    if (g_pStreamPrologue->audioChunkCount != 0) {
        g_15_pStreamerSoundChunks = (IxStreamerAudioChunk *)
            g_pIxMalloc(
                g_pStreamPrologue->audioChunkCount *
                sizeof(IxStreamerAudioChunk));
        if (g_15_pStreamerSoundChunks == 0) {
            ix_log_printf("Fatal [%s - %d]:\n", IX_STREAMER_FILE, 157);
            ix_log_printf("Streamer failed to allocate.");
            exit(-1);
        }
        ix_file_seek(g_1_pFeederPackageDisk,
                     g_pStreamPrologue->audioChunkTableOffset);
        ix_file_read(g_1_pFeederPackageDisk,
                     g_15_pStreamerSoundChunks,
                     g_pStreamPrologue->audioChunkCount *
                     sizeof(IxStreamerAudioChunk));
        g_dwStreamerMode |= IX_STREAMER_HAS_AUDIO;
    }
    if (g_pStreamPrologue->branchCount != 0) {
        g_7_pStreamBranches = (IxStreamerBranch *)
            g_pIxMalloc(g_pStreamPrologue->branchCount *
                                 sizeof(IxStreamerBranch));
        if (g_7_pStreamBranches == 0) {
            ix_log_printf("Fatal [%s - %d]:\n", IX_STREAMER_FILE, 168);
            ix_log_printf("Streamer failed to allocate.");
            exit(-1);
        }
        ix_file_seek(g_1_pFeederPackageDisk,
                     g_pStreamPrologue->branchTableOffset);
        ix_file_read(g_1_pFeederPackageDisk,
                     g_7_pStreamBranches,
                     g_pStreamPrologue->branchCount *
                     sizeof(IxStreamerBranch));
    }
    if (g_pStreamPrologue->triggerCount != 0) {
        g_6_pFeederTriggers = (IxStreamerTrigger *)
            g_pIxMalloc(g_pStreamPrologue->triggerCount *
                                 sizeof(IxStreamerTrigger));
        if (g_6_pFeederTriggers == 0) {
            ix_log_printf("Fatal [%s - %d]:\n", IX_STREAMER_FILE, 177);
            ix_log_printf("Streamer failed to allocate.");
            exit(-1);
        }
        ix_file_seek(g_1_pFeederPackageDisk,
                     g_pStreamPrologue->triggerTableOffset);
        ix_file_read(g_1_pFeederPackageDisk,
                     g_6_pFeederTriggers,
                     g_pStreamPrologue->triggerCount *
                     sizeof(IxStreamerTrigger));
    }
    if (g_pStreamPrologue->fileEntryCount != 0) {
        g_3_pReaderFileEntries = (IxStreamerFileEntry *)
            g_pIxMalloc(
                g_pStreamPrologue->fileEntryCount *
                sizeof(IxStreamerFileEntry));
        if (g_3_pReaderFileEntries == 0) {
            ix_log_printf("Fatal [%s - %d]:\n", IX_STREAMER_FILE, 186);
            ix_log_printf("Streamer failed to allocate.");
            exit(-1);
        }
        ix_file_seek(g_1_pFeederPackageDisk,
                     g_pStreamPrologue->fileEntryTableOffset);
        ix_file_read(g_1_pFeederPackageDisk,
                     g_3_pReaderFileEntries,
                     g_pStreamPrologue->fileEntryCount *
                     sizeof(IxStreamerFileEntry));
    }
    if (g_pStreamPrologue->fileChunkCount != 0) {
        g_17_pStreamerFileParts = (IxStreamerFileChunk *)
            g_pIxMalloc(
                g_pStreamPrologue->fileChunkCount *
                sizeof(IxStreamerFileChunk));
        if (g_17_pStreamerFileParts == 0) {
            ix_log_printf("Fatal [%s - %d]:\n", IX_STREAMER_FILE, 195);
            ix_log_printf("Streamer failed to allocate.");
            exit(-1);
        }
        ix_file_seek(g_1_pFeederPackageDisk,
                     g_pStreamPrologue->fileChunkTableOffset);
        ix_file_read(g_1_pFeederPackageDisk,
                     g_17_pStreamerFileParts,
                     g_pStreamPrologue->fileChunkCount *
                     sizeof(IxStreamerFileChunk));
    }
    if (g_pStreamPrologue->packetCount != 0) {
        g_7_pStreamerPacketOffsets = (unsigned int *)
            g_pIxMalloc(
                g_pStreamPrologue->packetCount *
                sizeof(unsigned int));
        if (g_7_pStreamerPacketOffsets == 0) {
            ix_log_printf("Fatal [%s - %d]:\n", IX_STREAMER_FILE, 204);
            ix_log_printf("Streamer failed to allocate.");
            exit(-1);
        }
        ix_file_seek(g_1_pFeederPackageDisk,
                     g_pStreamPrologue->packetTableOffset);
        ix_file_read(g_1_pFeederPackageDisk,
                     g_7_pStreamerPacketOffsets,
                     g_pStreamPrologue->packetCount *
                     sizeof(unsigned int));
    }
    if ((g_dwStreamerMode & IX_STREAMER_HAS_AUDIO) != 0) {
        g_49_nStreamerAudioBufferSize =
            g_pStreamPrologue->audioBufferSize;
        ix_dsp_configure(0x100,
            (void *)g_pStreamPrologue->dspConfigValue);
        ix_dsps_alloc(0, g_49_nStreamerAudioBufferSize,
                      g_pStreamPrologue->frequency,
                      g_pStreamPrologue->bitsPerSample,
                      g_pStreamPrologue->channels);
        ix_dsps_set_volume(0, g_nStreamerVolume);
        g_16_nStreamerBytesEachSecond =
            (g_pStreamPrologue->bitsPerSample >> 3) *
            g_pStreamPrologue->channels *
            g_pStreamPrologue->frequency;
    }
    g_126_pReaderCompressedBuf = (unsigned char *)
        g_pIxMalloc(g_pStreamPrologue->fileBufferSize);
    if (g_126_pReaderCompressedBuf == 0) {
        ix_log_printf("Fatal [%s - %d]:\n", IX_STREAMER_FILE, 238);
        ix_log_printf("Failed to allocate file_buffer!");
        exit(-1);
    }
    g_pStreamerFileStore = (unsigned char *)
        g_pIxMalloc(g_pStreamPrologue->fileBufferSize);
    if (g_pStreamerFileStore == 0) {
        ix_log_printf("Fatal [%s - %d]:\n", IX_STREAMER_FILE, 240);
        ix_log_printf("Failed to allocate file_buffer!");
        exit(-1);
    }
    g_dwStreamerMode |= IX_STREAMER_FILE_OPEN;
    SetEvent(g_1_hStreamRouseEvent);
    return 0;
}

/* Function start: 0x46C67A */   /* source line(s) 250;251: Streamer not ready to open stream file! | Stream file not open! */
extern "C" void ix_streamer_close_stream_file(void)
{
    IxStreamFile *streamFile;
    IxStreamFile *prev;

    if ((g_dwStreamerMode & IX_STREAMER_INITIALIZED) == 0) {
        ix_log_printf("Warning [%s - %d]:\n", IX_STREAMER_FILE, 250);
        ix_log_printf("Streamer not ready to open stream file!");
        return;
    }
    if ((g_dwStreamerMode & IX_STREAMER_FILE_OPEN) == 0) {
        ix_log_printf("Warning [%s - %d]:\n", IX_STREAMER_FILE, 251);
        ix_log_printf("Stream file not open!");
        return;
    }
    streamFile = g_48_pPumpIdleFiles;
    while (streamFile != 0) {
        prev = streamFile->previous;
        ix_streamer_close_file(streamFile);
        streamFile = prev;
    }
    EnterCriticalSection(&g_1_csStreamer);
    g_dwStreamerMode &= 0xffffff91;
    g_pIxFree(g_pStreamPrologue);
    g_pIxFree(g_15_pStreamerSoundChunks);
    g_pIxFree(g_7_pStreamBranches);
    g_pIxFree(g_6_pFeederTriggers);
    g_pIxFree(g_3_pReaderFileEntries);
    g_pIxFree(g_17_pStreamerFileParts);
    g_pIxFree(g_7_pStreamerPacketOffsets);
    g_pIxFree(g_126_pReaderCompressedBuf);
    g_pIxFree(g_pStreamerFileStore);
    ix_dsps_free(0);
    ix_file_close(g_1_pFeederPackageDisk);
    LeaveCriticalSection(&g_1_csStreamer);
    SetEvent(g_1_hStreamRouseEvent);
}

/* Function start: 0x46C7F3 */   /* source line(s) 286: Stream has no audio */
extern "C" void ix_streamer_audio_play(void)
{
    if ((g_dwStreamerMode & IX_STREAMER_HAS_AUDIO) == 0) {
        ix_log_printf("Warning [%s - %d]:\n", IX_STREAMER_FILE, 286);
        ix_log_printf("Stream has no audio");
        return;
    }
    g_dwStreamerMode |= IX_STREAMER_AUDIO_PLAYING;
    g_dwStreamerMode &= ~IX_STREAMER_AUDIO_PAUSED;
    SetEvent(g_1_hStreamRouseEvent);
}

/* Function start: 0x46C853 */   /* source line(s) 295: Stream has no audio */
extern "C" void ix_streamer_audio_stop(void)
{
    if ((g_dwStreamerMode & IX_STREAMER_HAS_AUDIO) == 0) {
        ix_log_printf("Warning [%s - %d]:\n", IX_STREAMER_FILE, 295);
        ix_log_printf("Stream has no audio");
        return;
    }
    g_dwStreamerMode &= ~IX_STREAMER_AUDIO_PLAYING;
    ix_streamer_seek_chunk(0);
    SetEvent(g_1_hStreamRouseEvent);
}

/* Function start: 0x46C8B6 */   /* source line(s) 304: Stream has no audio */
void ix_streamer_audio_pause(void)
{
    if ((g_dwStreamerMode & IX_STREAMER_HAS_AUDIO) == 0) {
        ix_log_printf("Warning [%s - %d]:\n", IX_STREAMER_FILE, 304);
        ix_log_printf("Stream has no audio");
        return;
    }
    g_dwStreamerMode |= IX_STREAMER_AUDIO_PAUSED;
    SetEvent(g_1_hStreamRouseEvent);
}

/* Function start: 0x46C90F */   /* source line(s) 312: Stream has no audio */
void ix_streamer_audio_reprepare(void)
{
    if ((g_dwStreamerMode & IX_STREAMER_HAS_AUDIO) == 0) {
        ix_log_printf("Warning [%s - %d]:\n", IX_STREAMER_FILE, 312);
        ix_log_printf("Stream has no audio");
        return;
    }
    g_dwStreamerMode |= IX_STREAMER_REPREPARE_AUDIO;
    SetEvent(g_1_hStreamRouseEvent);
}

/* Function start: 0x46C96B */
extern "C" void ix_streamer_set_intensity(unsigned char intensity)
{
    if (intensity >= 0) {
        g_6_bFeederLevel = intensity < 100 ? intensity : 100;
    } else {
        g_6_bFeederLevel = 0;
    }
}

/* Function start: 0x46C9AC */
unsigned char ix_streamer_get_intensity(void)
{
    return g_6_bFeederLevel;
}

/* Function start: 0x46C9C1 */
extern "C" void ix_streamer_set_trigger(char trigger)
{
    if (trigger == -1) {
        g_cStreamerBranchTag = -1;
    } else {
        if (trigger >= 0) {
            g_cStreamerBranchTag =
                trigger < 0x40 ? trigger : 0x40;
        } else {
            g_cStreamerBranchTag = 0;
        }
    }
}

/* Function start: 0x46CA19 */
char ix_streamer_get_trigger(void)
{
    return g_cStreamerBranchTag;
}

/* Function start: 0x46CA2E */   /* source line(s) 342: Stream has no audio */
extern "C" void ix_streamer_force_trigger(char trigger)
{
    unsigned int triggerCount;
    unsigned int chunkIndex;
    unsigned int audioChunks;
    unsigned int triggerIndex;

    if ((g_dwStreamerMode & IX_STREAMER_HAS_AUDIO) == 0) {
        ix_log_printf("Warning [%s - %d]:\n", IX_STREAMER_FILE, 342);
        ix_log_printf("Stream has no audio");
        return;
    }
    chunkIndex = g_6_dwStreamSoundChunk;
    audioChunks = g_pStreamPrologue->audioChunkCount;
    EnterCriticalSection(&g_1_csStreamer);
    while (-1 < trigger && audioChunks--) {
        if (g_15_pStreamerSoundChunks[chunkIndex].triggerCount > 0) {
            triggerCount =
                g_15_pStreamerSoundChunks[chunkIndex].triggerCount;
            triggerIndex =
                g_15_pStreamerSoundChunks[chunkIndex].firstTrigger;
            while (triggerCount--) {
                if (g_6_pFeederTriggers[triggerIndex].tag ==
                    trigger) {
                    g_13_adwPlayerBranchingStack[
                        g_nStreamerBranchStackIndex] =
                            g_6_dwStreamSoundChunk;
                    g_nStreamerBranchStackIndex =
                        (g_nStreamerBranchStackIndex + 1) & 0x1f;
                    g_6_dwStreamSoundChunk =
                        g_6_pFeederTriggers[
                            triggerIndex].audioChunk;
                    trigger = -1;
                    break;
                }
                triggerIndex++;
            }
        }
        chunkIndex++;
        if (g_pStreamPrologue->audioChunkCount - 1 <= chunkIndex)
            chunkIndex = 0;
    }
    if ((g_dwStreamerMode & IX_STREAMER_DSP_PLAYING) != 0) {
        ix_dsps_stop(0);
        g_dwStreamerMode &= ~IX_STREAMER_DSP_PLAYING;
    }
    LeaveCriticalSection(&g_1_csStreamer);
    SetEvent(g_1_hStreamRouseEvent);
}

/* Function start: 0x46CBBE */
extern "C" void ix_streamer_set_volume(unsigned short volume)
{
    if (volume >= 0) {
        g_nStreamerVolume =
            volume < 0xffff ? volume : 0xffff;
    } else {
        g_nStreamerVolume = 0;
    }
    if ((g_dwStreamerMode & 2) != 0) {
        EnterCriticalSection(&g_1_csStreamer);
        ix_dsps_set_volume(0, g_nStreamerVolume);
        LeaveCriticalSection(&g_1_csStreamer);
    }
}

/* Function start: 0x46CC3B */
extern "C" unsigned short ix_streamer_get_volume(void)
{
    return g_nStreamerVolume;
}

/* Function start: 0x46CC51 */
unsigned int ix_streamer_get_audio_chunk(void)
{
    return g_6_dwStreamSoundChunk;
}

/* Function start: 0x46CC66 */
void ix_streamer_seek_chunk(unsigned int chunk)
{
    EnterCriticalSection(&g_1_csStreamer);
    if (chunk >= 0) {
        g_6_dwStreamSoundChunk =
            g_pStreamPrologue->audioChunkCount - 1 < chunk
                ? g_pStreamPrologue->audioChunkCount - 1
                : chunk;
    } else {
        g_6_dwStreamSoundChunk = 0;
    }
    LeaveCriticalSection(&g_1_csStreamer);
}

/* Function start: 0x46CCC0 */
unsigned int ix_streamer_hash_name(unsigned char *name)
{
    int position;
    unsigned int character;
    unsigned int hash;

    position = 0;
    character = *name++;
    if (character >= 'a' && character <= 'z')
        character -= 0x20;
    hash = character << 8;
    while ((character = *name++) != 0) {
        if (character >= 'a' && character <= 'z')
            character -= 0x20;
        hash += (hash >> 4) * character;
        hash += position;
        position++;
    }
    return hash;
}

/* Function start: 0x46CD55 */
IxStreamerFileEntry *ix_streamer_find_entry(unsigned int hash)
{
    unsigned int limit;
    IxStreamerFileEntry *entry;
    unsigned int first;
    unsigned int mid;

    first = 0;
    limit = g_pStreamPrologue->fileEntryCount;
    while (limit > first) {
        mid = (first + limit) >> 1;
        entry = &g_3_pReaderFileEntries[mid];
        if (entry->nameHash < hash) {
            first = mid + 1;
        } else if (entry->nameHash > hash) {
            limit = mid;
        } else {
            return entry;
        }
    }
    return 0;
}

/* Function start: 0x46CDE3 */   /* source line(s) 512;526;535;549: open_file failed to allocate memory! | Failed to find_file in stream, %s! | open_file fail */
IxStreamFile *ix_streamer_open_file(unsigned char *name,
                                    unsigned char priority)
{
    IxStreamFile *streamFile;
    IxStreamerFileEntry *entry;
    FILE *file;

    streamFile = 0;
    if ((g_dwStreamerMode & IX_STREAMER_DEVELOPER_MODE) == 0) {
        entry = ix_streamer_find_entry(ix_streamer_hash_name(name));
        if (entry != 0) {
            streamFile = (IxStreamFile *)
                g_pIxMalloc(sizeof(IxStreamFile));
            if (streamFile == 0) {
                ix_log_printf("Fatal [%s - %d]:\n", IX_STREAMER_FILE, 512);
                ix_log_printf("open_file failed to allocate memory!");
                exit(-1);
            }
            streamFile->flags = IX_STREAM_FILE_OPEN;
            streamFile->entry = entry;
            streamFile->priority = priority;
            streamFile->position = 0;
            streamFile->completionEvent = CreateEventA(0, TRUE, FALSE, 0);
            streamFile->size = entry->size;
            EnterCriticalSection(&g_csStreamerFileFifo);
            streamFile->previous = g_48_pPumpIdleFiles;
            streamFile->next = 0;
            if (g_48_pPumpIdleFiles != 0)
                g_48_pPumpIdleFiles->next = streamFile;
            g_48_pPumpIdleFiles = streamFile;
            LeaveCriticalSection(&g_csStreamerFileFifo);
        } else {
            ix_log_printf("Warning [%s - %d]:\n", IX_STREAMER_FILE, 526);
            ix_log_printf("Failed to find_file in stream, %s!", name);
        }
    } else {
        file = ix_file_open((char *)name, 1);
        if (file != 0) {
            streamFile = (IxStreamFile *)
                g_pIxMalloc(sizeof(IxStreamFile));
            if (streamFile == 0) {
                ix_log_printf("Fatal [%s - %d]:\n", IX_STREAMER_FILE, 535);
                ix_log_printf("open_file failed to allocate memory!");
                exit(-1);
            }
            streamFile->flags = IX_STREAM_FILE_OPEN;
            streamFile->file = file;
            streamFile->size = ix_file_size(file);
            streamFile->priority = priority;
            streamFile->position = 0;
            streamFile->completionEvent = CreateEventA(0, TRUE, FALSE, 0);
            EnterCriticalSection(&g_csStreamerFileFifo);
            streamFile->previous = g_48_pPumpIdleFiles;
            streamFile->next = 0;
            if (g_48_pPumpIdleFiles != 0)
                g_48_pPumpIdleFiles->next = streamFile;
            g_48_pPumpIdleFiles = streamFile;
            LeaveCriticalSection(&g_csStreamerFileFifo);
        } else {
            ix_log_printf("Warning [%s - %d]:\n", IX_STREAMER_FILE, 549);
            ix_log_printf("Failed to find_file in stream, %s!", name);
        }
    }
    return streamFile;
}

/* Function start: 0x46D05D */
void ix_streamer_close_file(IxStreamFile *streamFile)
{
    streamFile->ix_stream_file_wait();
    EnterCriticalSection(&g_csStreamerFileFifo);
    if (streamFile->previous != 0)
        streamFile->previous->next = streamFile->next;
    if (streamFile->next == 0)
        g_48_pPumpIdleFiles = streamFile->previous;
    else
        streamFile->next->previous = streamFile->previous;
    LeaveCriticalSection(&g_csStreamerFileFifo);
    if ((g_dwStreamerMode & IX_STREAMER_DEVELOPER_MODE) != 0)
        ix_file_close(streamFile->file);
    CloseHandle(streamFile->completionEvent);
    g_pIxFree(streamFile);
}

/* Never referenced; retail reserves its storage after the thread tick. */
unsigned int g_38_nUnusedPumpWord; /* 0x005c4b40 */
