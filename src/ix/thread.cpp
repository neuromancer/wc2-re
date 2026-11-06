/*
 *  ix / D:\Rnd\Prj\ix\Src\Streamer\thread.cpp
 *
 *  Retail address range: 0x0046D110 - 0x0046DCFF (7 functions)
 *  Object boundaries follow retail padding and alignment; diagnostic paths
 *  identify the source subsystem (see docs/ORDER.md).
 *
 *  Implement in ADDRESS ORDER -- MSVC emits functions in source order, so the
 *  list below is the original source order of this file.
 */
#define IX_THREAD_CPP
#include "ix.h"
#include <stdlib.h>
#include <string.h>

#pragma function(memcpy)

#define IX_THREAD_FILE "D:\\Rnd\\Prj\\ix\\Src\\Streamer\\thread.cpp"

/* Function start: 0x46D110 */
DWORD WINAPI ix_streamer_thread_proc(void *parameter)
{
    DWORD timeout;

    timeout = 0;
    EnterCriticalSection(&g_49_csStreamWorker);
    g_dwStreamerMode |= IX_STREAMER_THREAD_RUNNING;
    while ((g_dwStreamerMode & IX_STREAMER_SHUTDOWN) == 0) {
        g_18_dwStreamerThreadTick++;
        EnterCriticalSection(&g_1_csStreamer);
        if ((g_dwStreamerMode & IX_STREAMER_FILE_OPEN) != 0) {
            if ((g_dwStreamerMode &
                 IX_STREAMER_REPREPARE_AUDIO) != 0) {
                if ((g_dwStreamerMode &
                     IX_STREAMER_DSP_PLAYING) != 0) {
                    ix_dsps_stop(0);
                    g_dwStreamerMode &=
                        ~IX_STREAMER_DSP_PLAYING;
                }
                ix_dsps_prepare(0);
                g_dwStreamerMode &=
                    ~(IX_STREAMER_REPREPARE_AUDIO |
                      IX_STREAMER_END_TRIGGERED);
            }
            if ((g_dwStreamerMode &
                 (IX_STREAMER_HAS_AUDIO | IX_STREAMER_AUDIO_PLAYING)) ==
                (IX_STREAMER_HAS_AUDIO | IX_STREAMER_AUDIO_PLAYING)) {
                timeout = ix_streamer_service_audio();
                if ((g_dwStreamerMode &
                     IX_STREAMER_AUDIO_PAUSED) != 0) {
                    if ((g_dwStreamerMode &
                         IX_STREAMER_DSP_PAUSED) == 0) {
                        ix_dsps_stop(0);
                        g_dwStreamerMode |=
                            IX_STREAMER_DSP_PAUSED;
                    }
                } else {
                    if ((g_dwStreamerMode &
                         IX_STREAMER_DSP_PAUSED) != 0) {
                        ix_dsps_play(0);
                        g_dwStreamerMode &=
                            ~IX_STREAMER_DSP_PAUSED;
                    }
                }
            } else {
                timeout = ix_thread_service_streams();
                if ((g_dwStreamerMode &
                     IX_STREAMER_DSP_PLAYING) != 0) {
                    ix_dsps_stop(0);
                    g_dwStreamerMode &=
                        ~(IX_STREAMER_DSP_PLAYING |
                          IX_STREAMER_DSP_PAUSED);
                }
            }
            LeaveCriticalSection(&g_1_csStreamer);
            WaitForSingleObject(g_1_hStreamRouseEvent, timeout);
            ResetEvent(g_1_hStreamRouseEvent);
        } else {
            if ((g_dwStreamerMode & IX_STREAMER_DSP_PLAYING) != 0) {
                ix_dsps_stop(0);
                g_dwStreamerMode &=
                    ~(IX_STREAMER_DSP_PLAYING | IX_STREAMER_DSP_PAUSED);
            }
            LeaveCriticalSection(&g_1_csStreamer);
            WaitForSingleObject(g_1_hStreamRouseEvent, INFINITE);
            ResetEvent(g_1_hStreamRouseEvent);
        }
    }
    g_dwStreamerMode &= ~IX_STREAMER_THREAD_RUNNING;
    LeaveCriticalSection(&g_49_csStreamWorker);
    (void)parameter;
    return 0;
}

/* Function start: 0x46D2C0 */
unsigned int ix_streamer_service_audio(void)
{
    unsigned int bytesUntilStop;

    if ((g_dwStreamerMode & IX_STREAMER_END_TRIGGERED) != 0) {
        bytesUntilStop = g_pStreamPrologue->audioBufferSize -
                         ix_dsps_get_buffer_free(0);
        Sleep((bytesUntilStop * 1000U) /
              g_16_nStreamerBytesEachSecond);
        g_dwStreamerMode &= 0xfffff7db;
        ix_dsps_stop(0);
    } else if ((g_dwStreamerMode &
                IX_STREAMER_DSP_PLAYING) == 0) {
        g_dwStreamerMode |= IX_STREAMER_DSP_PLAYING;
        ix_dsps_prepare(0);
        while (ix_thread_get_audio_chunk_size() <
               (unsigned int)ix_dsps_get_buffer_free(0))
            ix_thread_lock_stream_buffer();
        ix_dsps_play(0);
    } else {
        while (ix_thread_get_audio_chunk_size() <
               (unsigned int)ix_dsps_get_buffer_free(0))
            ix_thread_lock_stream_buffer();
    }
    return ix_thread_service_streams();
}

/* Function start: 0x46D3A6 */   /* source line(s) 196: File chunk without packet! */
void ix_thread_handle_file_chunk(IxStreamFile *streamFile)
{
    unsigned int archiveOffset;
    IxStreamerFileChunk *sourceChunk;
    unsigned int chunkIndex;
    unsigned int unreadChunks;
    IxStreamerFileEntry *directoryRecord;
    unsigned int offsetInPart;
    unsigned int partDataSize;
    unsigned char *readSource;
    unsigned int byteCount;
    unsigned int midpoint;
    unsigned int firstPacket;
    unsigned int *selectedPacket;
    unsigned int endPacket;

    directoryRecord = streamFile->entry;
    streamFile->serviceTick = g_18_dwStreamerThreadTick;
    if ((g_dwStreamerMode & 0x100) != 0) {
        ix_file_seek(streamFile->file, streamFile->position);
        ix_file_read(streamFile->file, streamFile->destination,
                     streamFile->remaining);
        EnterCriticalSection(&g_csStreamerFileFifo);
        streamFile->flags &= ~IX_STREAM_FILE_READING;
        if (streamFile->previous != 0)
            streamFile->previous->next = streamFile->next;
        if (streamFile->next == 0)
            g_1_pPlayerReadQueue = streamFile->previous;
        else
            streamFile->next->previous = streamFile->previous;
        streamFile->previous = g_48_pPumpIdleFiles;
        streamFile->next = 0;
        if (g_48_pPumpIdleFiles != 0)
            g_48_pPumpIdleFiles->next = streamFile;
        g_48_pPumpIdleFiles = streamFile;
        LeaveCriticalSection(&g_csStreamerFileFifo);
        SetEvent(streamFile->completionEvent);
        return;
    }

    chunkIndex = directoryRecord->firstChunk;
    unreadChunks = directoryRecord->chunkCount;
    sourceChunk = &g_17_pStreamerFileParts[chunkIndex];
    archiveOffset = (unsigned int)ix_file_tell(
        g_1_pFeederPackageDisk);
    while (unreadChunks--) {
        if (sourceChunk->fileOffset <= streamFile->position &&
            sourceChunk->fileEnd > streamFile->position) {
            partDataSize = sourceChunk->fileEnd - sourceChunk->fileOffset;
            if (chunkIndex != g_nSpoolerDiskChunk) {
                firstPacket = sourceChunk->firstPacket;
                endPacket = sourceChunk->packetCount + firstPacket;
                selectedPacket = 0;
                g_nSpoolerDiskChunk = chunkIndex;
                while (endPacket > firstPacket) {
                    midpoint = (endPacket + firstPacket) >> 1;
                    selectedPacket = &g_7_pStreamerPacketOffsets[midpoint];
                    if (*selectedPacket < archiveOffset)
                        firstPacket = midpoint + 1;
                    else if (*selectedPacket > archiveOffset)
                        endPacket = midpoint;
                    else
                        break;
                }
                if (selectedPacket == 0) {
                    ix_log_printf("Fatal [%s - %d]:\n", IX_THREAD_FILE, 196);
                    ix_log_printf("File chunk without packet!");
                    exit(-1);
                }
                if (sourceChunk->packedSize < 0) {
                    ix_file_seek(g_1_pFeederPackageDisk, *selectedPacket);
                    ix_file_read(g_1_pFeederPackageDisk,
                                 g_126_pReaderCompressedBuf,
                                 -sourceChunk->packedSize);
#ifdef SDL_PORT
                    ix_lzo1x_decompress(
                        g_126_pReaderCompressedBuf,
                        g_pStreamerFileStore, partDataSize,
                        (unsigned int)-sourceChunk->packedSize);
#else
                    ix_lzo1x_decompress(g_126_pReaderCompressedBuf,
                                        g_pStreamerFileStore,
                                        partDataSize);
#endif
                } else {
                    ix_file_seek(g_1_pFeederPackageDisk, *selectedPacket);
                    ix_file_read(g_1_pFeederPackageDisk,
                                 g_pStreamerFileStore,
                                 sourceChunk->packedSize);
                }
            }
            readSource = g_pStreamerFileStore;
            byteCount = streamFile->remaining;
            offsetInPart = streamFile->position - sourceChunk->fileOffset;
            partDataSize = partDataSize - offsetInPart;
            readSource += offsetInPart;
            if (byteCount > partDataSize)
                byteCount = partDataSize;
            memcpy(streamFile->destination, readSource, byteCount);
            streamFile->destination += byteCount;
            streamFile->position += byteCount;
            streamFile->remaining = streamFile->remaining - byteCount;
            if (streamFile->remaining == 0) {
                EnterCriticalSection(&g_csStreamerFileFifo);
                if (streamFile->previous != 0)
                    streamFile->previous->next = streamFile->next;
                if (streamFile->next == 0)
                    g_1_pPlayerReadQueue = streamFile->previous;
                else
                    streamFile->next->previous = streamFile->previous;
                streamFile->previous = g_48_pPumpIdleFiles;
                streamFile->next = 0;
                if (g_48_pPumpIdleFiles != 0)
                    g_48_pPumpIdleFiles->next = streamFile;
                g_48_pPumpIdleFiles = streamFile;
                streamFile->flags &= ~IX_STREAM_FILE_READING;
                LeaveCriticalSection(&g_csStreamerFileFifo);
                SetEvent(streamFile->completionEvent);
            }
            return;
        }
        sourceChunk++;
        chunkIndex++;
    }
}

int  ix_dsps_get_buffer_free(int stream);
void ix_dsps_unlock(int stream);

/* Function start: 0x46D7C6 */
unsigned int ix_thread_service_streams(void)
{
    int bytesUntilRefill;
    IxStreamFile *streamFile;
    IxStreamFile *chosenFile;

    EnterCriticalSection(&g_csStreamerFileFifo);
    if (g_1_pPlayerReadQueue != 0) {
        streamFile = g_1_pPlayerReadQueue;
        chosenFile = 0;
        while (streamFile != 0) {
            if (chosenFile != 0) {
                if (chosenFile->priority < streamFile->priority)
                    chosenFile = streamFile;
                else if (streamFile->serviceTick < chosenFile->serviceTick)
                    chosenFile = streamFile;
            } else
                chosenFile = streamFile;
            streamFile = streamFile->previous;
        }
        LeaveCriticalSection(&g_csStreamerFileFifo);
        if (chosenFile != 0)
            ix_thread_handle_file_chunk(chosenFile);
    } else {
        LeaveCriticalSection(&g_csStreamerFileFifo);
    }

    if (g_1_pPlayerReadQueue != 0)
        return 0;
    if ((g_dwStreamerMode & 4) == 0 ||
        (g_dwStreamerMode & 8) != 0)
        return 1000;
    bytesUntilRefill = ix_thread_get_audio_chunk_size() -
                       ix_dsps_get_buffer_free(0);
    if (bytesUntilRefill > 0) {
        bytesUntilRefill += 0x16f8;
        return (bytesUntilRefill * 1000U) /
               g_16_nStreamerBytesEachSecond;
    }
    return 0;
}

/* Function start: 0x46D916 */
void ix_thread_advance_audio_chunk(void)
{
    if (g_15_pStreamerSoundChunks[
            g_6_dwStreamSoundChunk].triggerCount > 0) {
        unsigned int triggerIndex;
        unsigned int triggerCount;

        triggerCount = g_15_pStreamerSoundChunks[
            g_6_dwStreamSoundChunk].triggerCount;
        triggerIndex = g_15_pStreamerSoundChunks[
            g_6_dwStreamSoundChunk].firstTrigger;
        while (triggerCount--) {
            if (g_6_pFeederTriggers[triggerIndex].tag == 'A') {
                g_nStreamerBranchStackIndex =
                    (g_nStreamerBranchStackIndex - 1) & 0x1f;
                g_6_dwStreamSoundChunk =
                    g_13_adwPlayerBranchingStack[
                        g_nStreamerBranchStackIndex];
                return;
            } else if (g_6_pFeederTriggers[triggerIndex].tag ==
                       '@') {
                g_dwStreamerMode |= 0x800;
            } else if (g_6_pFeederTriggers[triggerIndex].tag ==
                       g_cStreamerBranchTag) {
                g_13_adwPlayerBranchingStack[
                    g_nStreamerBranchStackIndex] =
                        g_6_dwStreamSoundChunk;
                g_nStreamerBranchStackIndex =
                    (g_nStreamerBranchStackIndex + 1) & 0x1f;
                g_6_dwStreamSoundChunk =
                    g_6_pFeederTriggers[triggerIndex].audioChunk;
                g_cStreamerBranchTag = -1;
                return;
            }
            triggerIndex++;
        }
    }

    if (g_15_pStreamerSoundChunks[
            g_6_dwStreamSoundChunk].branchCount > 0) {
        unsigned int branch;
        unsigned int count;
        unsigned int pick;
        unsigned int bestGap;
        unsigned int gap;

        count = g_15_pStreamerSoundChunks[
            g_6_dwStreamSoundChunk].branchCount - 1;
        branch = g_15_pStreamerSoundChunks[
            g_6_dwStreamSoundChunk].firstBranch;
        bestGap =
            ((g_7_pStreamBranches[branch].intensity -
              g_6_bFeederLevel) >> 31 ^
             (g_7_pStreamBranches[branch].intensity -
              g_6_bFeederLevel)) -
            ((g_7_pStreamBranches[branch].intensity -
              g_6_bFeederLevel) >> 31);
        pick = branch;
        branch++;
        while (count--) {
            gap =
                ((g_7_pStreamBranches[branch].intensity -
                  g_6_bFeederLevel) >> 31 ^
                 (g_7_pStreamBranches[branch].intensity -
                  g_6_bFeederLevel)) -
                ((g_7_pStreamBranches[branch].intensity -
                  g_6_bFeederLevel) >> 31);
            if (gap < bestGap) {
                bestGap = gap;
                pick = branch;
            }
            branch++;
        }
        g_6_dwStreamSoundChunk =
            g_7_pStreamBranches[pick].audioChunk;
        return;
    }
    g_6_dwStreamSoundChunk++;
    if (g_pStreamPrologue->audioChunkCount - 1 <=
        g_6_dwStreamSoundChunk)
        g_6_dwStreamSoundChunk = 0;
    return;
}

/* Function start: 0x46DBC9 */   /* source line(s) 394: failed to lock stream buffer */
void ix_thread_lock_stream_buffer(void)
{
    unsigned int bytesToCopy;
    unsigned int lockedBytes;
    unsigned char *data;
    unsigned int fileOffset;

    bytesToCopy = ix_thread_get_audio_chunk_size();
    if (bytesToCopy > 0) {
        fileOffset = g_15_pStreamerSoundChunks[
            g_6_dwStreamSoundChunk].fileOffset;
        while (bytesToCopy > 0) {
            ix_dsps_lock(0, bytesToCopy, &data, &lockedBytes);
            if (data == 0) {
                ix_log_printf("Fatal [%s - %d]:\n", IX_THREAD_FILE, 394);
                ix_log_printf("failed to lock stream buffer");
                exit(-1);
            }
            ix_file_seek(g_1_pFeederPackageDisk, fileOffset);
            ix_file_read(g_1_pFeederPackageDisk, data,
                         lockedBytes);
            bytesToCopy = bytesToCopy - lockedBytes;
            fileOffset += lockedBytes;
            ix_dsps_unlock(0);
        }
    }
    ix_thread_advance_audio_chunk();
}

/* Function start: 0x46DCA6 */
unsigned int ix_thread_get_audio_chunk_size(void)
{
    unsigned int fileOffset;

    if (g_6_dwStreamSoundChunk != (unsigned int)-1) {
        fileOffset = g_15_pStreamerSoundChunks[
            g_6_dwStreamSoundChunk].fileOffset;
        return g_15_pStreamerSoundChunks[
            g_6_dwStreamSoundChunk].fileEnd - fileOffset;
    }
    return 0;
}
